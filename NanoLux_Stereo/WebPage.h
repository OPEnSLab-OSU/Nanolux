//
// Created by david on 11/10/2022.
//

#ifndef NANOLUX_WEBPAGE_H
#define NANOLUX_WEBPAGE_H

String web_page(){
    String page = "<!DOCTYPE html> <html>\n";
    page +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    page +="<title>LED Strip Color Control</title>\n";
    page +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    page +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    page +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
    page +=".button-blue {background-color: #0000cc;}\n";
    page +=".button-blue:active {background-color: #0000ff;}\n";
    page +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
    page +="</style>\n";
    page +="</head>\n";
    page +="<body>\n";
    page +="<h1>AudioLux Web Server</h1>\n";
    page +="<h3>Using Access Point(AP) Mode</h3>\n";

    page +="<a class=\"button button-blue\" href=\"/blank\">Blank</a>\n";
    page +="<a class=\"button button-blue\" href=\"/trail\">Trail</a>\n";
    page +="<a class=\"button button-blue\" href=\"/solid\">Solid</a>\n";
    page +="<a class=\"button button-blue\" href=\"/confetti\">Confetti</a>\n";
    page +="<a class=\"button button-blue\" href=\"/vbar\">Vol Bar</a>\n";

    page +="</body>\n";
    page +="</html>\n";
    return page;
}

#endif //NANOLUX_WEBPAGE_H
