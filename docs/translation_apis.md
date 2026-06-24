# Translation APIs

This document outlines the REST HTTP endpoints, request payloads, URL parameters, and authentication headers extracted from the ScreenTranslator JavaScript translators (`google_api.js`, `google.js`, and `baidu.js`). These are documented for the transition to native C++ Network Clients and WinRT APIs.

## Google API (`google_api.js`)

This uses a direct REST GET request to the Google API.

*   **Endpoint:** `https://translate.googleapis.com/translate_a/single`
*   **Method:** `GET`
*   **Authentication Headers:** None explicitly required.
*   **URL Parameters:**
    *   `client`: `gtx` (Constant)
    *   `sl`: `auto` (Source Language, hardcoded to auto)
    *   `tl`: Target Language code (e.g., `en`, `es`)
    *   `dt`: `t` (Constant)
    *   `q`: URL-encoded text to translate
*   **Request Payload:** None (Data is passed in URL parameters)

## Google Web (`google.js`)

This translator previously relied on a web scraper approach via browser automation (loading a URL and interacting with the DOM). The URL structure used for loading the page is documented below.

*   **Endpoint:** `https://translate.google.com/`
*   **Method:** Navigation (GET)
*   **Authentication Headers:** Standard browser headers.
*   **URL Parameters (Hash/Fragment):**
    *   `view`: `home`
    *   `op`: `translate`
    *   `sl`: `auto` (Source Language)
    *   `tl`: Target Language code
    *   `text`: URL-encoded text to translate
*   **Request Payload:** None. Data is passed via URL fragment and DOM manipulation.
*   **DOM Interaction details (For reference):**
    *   Input element: `textarea.er8xn`
    *   Output elements: `span.HwtZe > span > span`

## Baidu Web (`baidu.js`)

Similar to `google.js`, this translator relied on browser automation against the Baidu Translate web interface.

*   **Endpoint:** `https://fanyi.baidu.com/`
*   **Method:** Navigation (GET)
*   **Authentication Headers:** Standard browser headers.
*   **URL Parameters (Hash/Fragment):** The parameters are passed directly as a hash path.
    *   Format: `#{from}/{to}/{text}`
    *   `from`: Source Language code
    *   `to`: Target Language code
    *   `text`: URL-encoded text to translate
*   **Request Payload:** None. Data is passed via URL fragment and DOM manipulation.
*   **DOM Interaction details (For reference):**
    *   Input element: `textarea#baidu_translate_input`
    *   Output elements: `p.target-output`
