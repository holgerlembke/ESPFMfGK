// inline guard. Still broken by design?
#ifndef ESPFMfGKWpF2_h
#define ESPFMfGKWpF2_h

static const char ESPFMfGKWpFormIntro[] PROGMEM = 
R"==x==(<form><textarea id="tect" rows="25" cols="80">)==x==";


static const char ESPFMfGKWpFormExtro1[] PROGMEM = 
R"==x==(</textarea></form><button class="ed" title="Save file" onclick="sved(')==x==";

// not sure what the <script> part is for.
static const char ESPFMfGKWpFormExtro2[] PROGMEM = 
R"==x==(');" >Save</button>&nbsp;<button class="ed" title="Abort editing" onclick="abed();">Abort editing</button>

<script id="info">document.getElementById('o3').innerHTML = "File:";</script>)==x==";

#endif
