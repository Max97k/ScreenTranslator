# Translation APIs

This document outlines the translation API architecture used in ScreenTranslator.

## Google Cloud Translation API (v4.0.0+)

Starting from **v4.0.0**, all old JavaScript scraper translators (`google.js`, `google_api.js`, `baidu.js`) and the heavy `QWebEngine` browser backend have been completely removed.

Translation is now handled natively in C++ via `QNetworkAccessManager` using the official Google Cloud Translation API v2 POST endpoint.

### Request Details
*   **Endpoint:** `https://translation.googleapis.com/language/translate/v2`
*   **Method:** `POST`
*   **Authentication:** Authenticated via the API Key appended as a URL query parameter: `?key=YOUR_API_KEY`
*   **Headers:**
    *   `Content-Type`: `application/json`
*   **Request Payload (JSON):**
    ```json
    {
      "q": "Text to translate",
      "source": "en",
      "target": "zh-TW",
      "format": "text"
    }
    ```

### Response Parsing
The JSON response returned by the API is parsed directly:
```json
{
  "data": {
    "translations": [
      {
        "translatedText": "翻譯後的文字"
      }
    ]
  }
}
```
The application extracts `translatedText` from the first element in the `translations` array.
