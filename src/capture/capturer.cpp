#include "capturer.h"
#include "capturearea.h"
#include "captureareaselector.h"
#include "debug.h"
#include "manager.h"
#include "settings.h"
#include "task.h"

#include <QApplication>
#include <QPainter>
#include <QScreen>

#ifdef Q_OS_WIN

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <d3d11_4.h>
#include <dxgi1_2.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <windows.graphics.capture.interop.h>

using namespace winrt;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;

inline auto CreateDirect3DDevice(IDXGIDevice* dxgi_device)
{
    com_ptr<::IInspectable> d3d_device;
    check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
    return d3d_device.as<IDirect3DDevice>();
}

inline auto CreateCaptureItemForMonitor(HMONITOR hmonitor)
{
    auto activation_factory = get_activation_factory<GraphicsCaptureItem>();
    auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
    GraphicsCaptureItem item = { nullptr };
    check_hresult(interop_factory->CreateForMonitor(hmonitor, guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), put_abi(item)));
    return item;
}

struct CaptureContext {
    std::vector<QRect> screenRects;
    QRect fullRect;
    QPainter* painter;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM dwData)
{
    CaptureContext* ctx = reinterpret_cast<CaptureContext*>(dwData);

    QRect rect(lprcMonitor->left, lprcMonitor->top, 
               lprcMonitor->right - lprcMonitor->left, 
               lprcMonitor->bottom - lprcMonitor->top);
    ctx->screenRects.push_back(rect);
    ctx->fullRect |= rect;

    try {
        auto item = CreateCaptureItemForMonitor(hMonitor);
        
        com_ptr<ID3D11Device> d3dDevice;
        com_ptr<ID3D11DeviceContext> d3dContext;
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, 1, D3D11_SDK_VERSION, d3dDevice.put(), nullptr, d3dContext.put()));
        
        auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
        auto device = CreateDirect3DDevice(dxgiDevice.get());
        
        auto framePool = Direct3D11CaptureFramePool::CreateFreeThreaded(
            device,
            DirectXPixelFormat::B8G8R8A8UIntNormalized,
            1,
            item.Size());
        
        auto session = framePool.CreateCaptureSession(item);
        
        HANDLE frameEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        Direct3D11CaptureFrame frame = nullptr;
        
        auto eventToken = framePool.FrameArrived([&frame, frameEvent](auto& pool, auto&) {
            frame = pool.TryGetNextFrame();
            SetEvent(frameEvent);
        });
        
        session.StartCapture();
        WaitForSingleObject(frameEvent, 1000);
        
        // Remove the event handler to prevent it firing after stack variables are destroyed.
        framePool.FrameArrived(eventToken);
        session.Close();
        framePool.Close();
        
        if (frame) {
            auto surface = frame.Surface();
            auto access = surface.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
            com_ptr<ID3D11Texture2D> texture;
            check_hresult(access->GetInterface(guid_of<ID3D11Texture2D>(), texture.put_void()));
            
            D3D11_TEXTURE2D_DESC desc;
            texture->GetDesc(&desc);
            desc.Usage = D3D11_USAGE_STAGING;
            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.MiscFlags = 0;
            
            com_ptr<ID3D11Texture2D> stagingTexture;
            check_hresult(d3dDevice->CreateTexture2D(&desc, nullptr, stagingTexture.put()));
            d3dContext->CopyResource(stagingTexture.get(), texture.get());
            
            D3D11_MAPPED_SUBRESOURCE mapped;
            check_hresult(d3dContext->Map(stagingTexture.get(), 0, D3D11_MAP_READ, 0, &mapped));
            
            QImage img((const uchar*)mapped.pData, desc.Width, desc.Height, mapped.RowPitch, QImage::Format_ARGB32_Premultiplied);
            QImage copy = img.copy();
            d3dContext->Unmap(stagingTexture.get(), 0);
            
            ctx->painter->drawImage(rect, copy);
        }
        CloseHandle(frameEvent);
    } catch (...) {
        // Silently fail and it will not be drawn.
    }

    return TRUE;
}

#endif

Capturer::Capturer(Manager &manager, const Settings &settings,
                   const CommonModels &models)
  : manager_(manager)
  , settings_(settings)
  , selector_(std::make_unique<CaptureAreaSelector>(*this, settings_, models,
                                                    pixmap_, pixmapOffset_))
{
}

Capturer::~Capturer() = default;

void Capturer::capture()
{
  updatePixmap();
  SOFT_ASSERT(selector_, return );
  selector_->activate();
}

bool Capturer::canCaptureLocked()
{
  SOFT_ASSERT(selector_, return false);
  return selector_->hasLocked();
}

void Capturer::captureLocked()
{
  updatePixmap();
  SOFT_ASSERT(selector_, return );
  selector_->captureLocked();
}

void Capturer::updatePixmap()
{
#ifdef Q_OS_WIN
  CaptureContext ctx;

  QPixmap combined(1, 1); // Temporary size
  ctx.painter = nullptr;
  
  // First pass to determine the total size
  EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR, HDC, LPRECT lprcMonitor, LPARAM dwData) -> BOOL {
      CaptureContext* ctx = reinterpret_cast<CaptureContext*>(dwData);
      QRect rect(lprcMonitor->left, lprcMonitor->top, 
                 lprcMonitor->right - lprcMonitor->left, 
                 lprcMonitor->bottom - lprcMonitor->top);
      ctx->fullRect |= rect;
      return TRUE;
  }, reinterpret_cast<LPARAM>(&ctx));

  combined = QPixmap(ctx.fullRect.size());
  combined.fill(Qt::black);
  
  QPainter p(&combined);
  p.translate(-ctx.fullRect.topLeft());
  ctx.painter = &p;

  // Second pass to capture and draw
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&ctx));

  SOFT_ASSERT(selector_, return );
  pixmap_ = combined;
  pixmapOffset_ = ctx.fullRect.topLeft();

  for (auto &r : ctx.screenRects) r.translate(-ctx.fullRect.topLeft());
  selector_->setScreenRects(ctx.screenRects);
#else
  const auto screens = QApplication::screens();
  std::vector<QRect> screenRects;
  screenRects.reserve(screens.size());
  QRect rect;

  for (const QScreen *screen : screens) {
    const auto geometry = screen->geometry();
    screenRects.push_back(geometry);
    rect |= geometry;
  }

  QPixmap combined(rect.size());
  QPainter p(&combined);
  p.translate(-rect.topLeft());

  for (const auto screen : screens) {
    const auto geometry = screen->geometry();
    const auto pixmap =
        screen->grabWindow(0, 0, 0, geometry.width(), geometry.height());
    p.drawPixmap(geometry, pixmap);
  }

  SOFT_ASSERT(selector_, return );
  pixmap_ = combined;
  pixmapOffset_ = rect.topLeft();

  for (auto &r : screenRects) r.translate(-rect.topLeft());
  selector_->setScreenRects(screenRects);
#endif
}

void Capturer::repeatCapture()
{
  SOFT_ASSERT(selector_, return );
  selector_->activate();
}

void Capturer::updateSettings()
{
  SOFT_ASSERT(selector_, return );
  selector_->updateSettings();
}

void Capturer::selected(const CaptureArea &area)
{
  SOFT_ASSERT(selector_, return manager_.captureCanceled())
  selector_->hide();

  SOFT_ASSERT(!pixmap_.isNull(), return manager_.captureCanceled())
  auto task = area.task(pixmap_, pixmapOffset_);
  if (task)
    manager_.captured(task);
  else
    manager_.captureCanceled();
}

void Capturer::canceled()
{
  SOFT_ASSERT(selector_, return );
  selector_->hide();
  manager_.captureCanceled();
}
