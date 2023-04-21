#ifndef API_H
#define API_H


/*
 * Use https://arduinojson.org/v6/assistant/ to calculate the size 
 * required for the StaticJsonDocument.
 */

using namespace ARDUINOJSON_NAMESPACE;


/*
 * HTTP_OK, etc.
 * CONTENT_JSON, etc. come from WebServer.h
 */


/*
 * Handle Pattern request or selection
 * (Single-value API example.)
 * Assume payload is JSON:
 * { "patten": "<80-char-max-string>" }
 */
inline void handle_pattern_request() {
    if (webServer.method() == HTTP_PUT) {
        // Max pattern name length : 80 characters (and add overhead)
        StaticJsonDocument<128> payload;
        int args = webServer.args();

        // The JSON payload of the PUT statement lives in the "plain" property
        // of the request.
        const DeserializationError error = deserializeJson(payload, webServer.arg("plain"));

        // If there is a problem (unlikely for pattern since we provided the value
        // and it is, presumably, elected from a combo, but you never know), log
        // and tell the client the messed up.
        if (error) {
            Serial.print(F("Parsing pattern name failed: "));
            Serial.println(error.c_str());
            webServer.send(HTTP_UNPROCESSABLE, CONTENT_JSON, error.c_str());
            return;
        }

        int status = HTTP_OK;

        // Syntax error here, since there is no such variable (pattern).
        // In practice we will be altering a global (structure?) that
        // holds the current pattern value.

        // Note that, as is, pattern would have to be a char*. If it
        // is something else, then a conversion is required.
        pattern = payload["pattern"];


        Serial.println("Pattern handler: sending response back.");
        webServer.send(HTTP_OK, CONTENT_TEXT, build_response(true, "Pattern set.", nullptr));
    }
    else {
        // Same consideration applies for typeof(pattern)
        const String response = String("{ \"pattern\": ") + "\"" + String(pattern) + String("\" }");
        webServer.send(HTTP_OK, CONTENT_JSON, response);
    }
}


/*
 * Handle Settings request
 * (Simple value list API example.)
 */
inline void handle_patterns_list() {
    // Based on current swagger, which is probably outdated for
    // the audio/video part. This is to hold the current settings structure
    // which looks like this:
    // {
    //    "noise": 100,
    //    "compression" : 100,
    //    "loFreqHue" : "0xFFFFFF",
    //    "hiFreqHue" : "0xFFFFFF",
    //    "ledCount" : 100
    // }
    //
    // (Again, this probably not useful anymore, but serves as an example.
    //
    StaticJsonDocument<192> av_settings;

    // Same blurb about these variables.
    // Note that the JSON document automagically takes
    // the type of what you assign to it.
    av_settings["noise"] = noise_gate;
    av_settings["compression"] = compression;
    av_settings["loFreqHue"] = lo_freq_hue;
    av_settings["hiFreqHue"] = hi_freq_hue;
    av_settings["ledCount"] = led_count;

    String response;
    serializeJson(av_settings, response);

    webServer.send(HTTP_OK, CONTENT_JSON, response);
}


/*
 * WeBServer.h has more examples of API, like sending a bare value (no JSON)
 * or sending a list of values.
 */


#endif // API_H