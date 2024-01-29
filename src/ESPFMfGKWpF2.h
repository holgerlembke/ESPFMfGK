#ifndef ESPFMfGKWpF2_h
#define ESPFMfGKWpF2_h

// !! hat fehlenden > am Ende, weil noch Charset eingefügt wird
static const char ESPFMfGKWpFormIntro1[] PROGMEM =  
R"==x==(<form )==x==";

static const char ESPFMfGKWpFormIntro2[] PROGMEM =  
R"==x==(><div class="editor"><div class="line-numbers"><span></span></div><textarea class="windowcontent" rows="25" cols="80">)==x==";

static const char ESPFMfGKWpFormExtro1[] PROGMEM = 
R"==x==(</textarea></div></form>)==x==";

// not sure what the <script> part is for.
static const char ESPFMfGKWpFormExtro2[] PROGMEM = 
R"==x==(');" >Save</button>&nbsp;<button class="ed" title="Abort editing" onclick="abed();">Abort editing</button>

<script id="info">document.getElementById('o3').innerHTML = "File:";</script>)==x==";

#endif
