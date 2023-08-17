// inline guard. Did I mention that c/c++ is broken by design?
#ifndef ESPFMfGKWp2_h
#define ESPFMfGKWp2_h

// this file has been created by makeESPxWebFlMgrWp\do.cmd

//*****************************************************************************************************
static const char ESPFMfGKWpindexpage[] PROGMEM = R"==x==(
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>FileManager</title>
    <meta charset="utf-8"/>
    <link rel="stylesheet" type="text/css" href="/fm.css">
    <script src="/fm.js"></script>  
  </head>
  <body class="background">
    <div id="gc">
        <div class="o1">&nbsp;</div>
        <!-- <div class="o2" id="o2">&nbsp;</div> -->
        <div class="o2" id="o2">&nbsp;</div>        
        <div class="o3" id="o3">&nbsp;</div>
        <div class="o4">&nbsp;</div>

        <div class="m1">
            <div class="s11">&nbsp;</div>
            <div class="s12">
            <div class="s13 background">&nbsp;</div>
            </div>
        </div>
        <div class="m2" ondrop="dropHandler(event);" ondragover="dragOverHandler(event);">
          File<br />
          Drop<br />
          Zone<br />
        </div>
        <div class="m3">
            <div class="s31">&nbsp;</div>
            <div class="s32">
            <div class="s33 background">&nbsp;</div>
            </div>
        </div>

        <div class="u1">&nbsp;</div>
        <div class="u2" onclick="downloadall();">Download all files</div>
        <div class="u3" id="msg">Loading...</div>
        <div class="u4">&nbsp;</div>
        <div class="c" id="fi">
          File list should appear here.
        </div>
    </div>
    <div id="foot"></div>
  </body>
</html>  

  )==x==";

static const char ESPFMfGKWpjavascript[] PROGMEM = R"==x==(

var elemento2 = null;
var elemento3 = null;
var elementmsg = null;
var elementfi = null;

//000000000000000000000000000
function compressurlfile(source) {
  // dynamic post load js    
  var se = document.createElement('script');
  se.type = 'text/javascript';
  se.src = "/gzipper.js";
  document.head.appendChild(se);
    
  msgline("Fetching file...");
  var request = new XMLHttpRequest();
  request.onprogress = function(evt) {
    if (evt.lengthComputable) {
      var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
      if (lastpercentComplete != percentComplete) {
        lastpercentComplete = percentComplete;
        msgline("Fetching file "+ percentComplete + "%...");
      }
    } 
  }
  request.onreadystatechange = function() {
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
      var data = this.responseText;
      var gzip = require('gzip-js'),
        options = {
          level: 9,
          name: source,
          timestamp: parseInt(Date.now() / 1000, 10)
        };
      var out = gzip.zip(data, options);
      var bout = new Uint8Array(out); // out is 16 bits...

      msgline("Sending compressed file...");
      var sendback = new XMLHttpRequest();
      sendback.upload.onprogress = function(evt) {
        if (evt.lengthComputable) {
          var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
          if (lastpercentComplete != percentComplete) {
            lastpercentComplete = percentComplete;
            msgline("Sending compressed file "+ percentComplete + "%...");
          }
        } 
      }
      sendback.onreadystatechange = function() {
        var DONE = this.DONE || 4;
        if (this.readyState === DONE) {
          getfileinsert();
        }
      };
      sendback.open('POST', '/r?fs=' + getFileSystemIndex() + '&fn=' + source + '.gz');
      //sendback.open('POST', '/r');
      var formdata = new FormData();
      var blob = new Blob([bout], {
        type: "application/octet-binary"
      });
      formdata.append(source + '.gz', blob, source + '.gz');
      sendback.send(formdata);
    }
  };
  request.open('GET', source + "?fs="+ getFileSystemIndex(), true);
  request.send(null);
}

//000000000000000000000000000
function getFileSystemIndex() {
  var selectinput = document.getElementById('memory');
  if (selectinput == null) {
    console.log('Hint: get default file system index');
    return -1;
  } else {
    return selectinput.selectedIndex;
  }
}

//000000000000000000000000000
function executecommand(command) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
      getfileinsert();
    }
  };
  xhr.open('GET', '/job?fs=' + getFileSystemIndex() + "&" + command, true);
  xhr.send(null);
}

//000000000000000000000000000
function getfileinsert() {
  var param = getFileSystemIndex();

  msgline("Fetching files infos...");
  var request = new XMLHttpRequest();
  request.onprogress = function(evt) {
/*        
      var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
      if (lastpercentComplete != percentComplete) {
        lastpercentComplete = percentComplete;
*/        
     msgline("Fetching files infos, "+evt.loaded + "  B received.");
     // console.log('Info: '+evt.loaded);
  }
  request.onreadystatechange = AnswerProcessor;
  request.open('GET', '/i?fs=' + param, true);
  request.send(null);
}

//000000000000000000000000000
// Handle the getfileinsert answer
function AnswerProcessor() {
  var DONE = this.DONE || 4;
  if (this.readyState === DONE) {
    var antworttrenner = String.fromCharCode(02, 01, 03);
    var itemtrenner = String.fromCharCode(02, 01, 04);
    
    var res = this.responseText.split(antworttrenner);

    elemento3.innerHTML = res[1];
    elemento2.innerHTML = res[2];

    var items = res[0].split(itemtrenner);
    var itemhtml = "<div class=\"cc\"><div class=\"gc\">";

    var c = 0;
    while (c < items.length - 3) {
      var s = "";

      // Flags, sync with class ESPFMfGK
      var flags = parseInt(items[c + 4]);
      // s += flags.toString(2);

      if ((flags & (1 << 5)) != 0) { // flagcandownload
        s += "<div class=\"ccl %cc\" onclick=\"downloadfile(\'%fn\')\">"+
             "<span title=\"Download\">"+
             "&nbsp;&nbsp;%fd&nbsp;"+
             "</span></div>";
      } else {
        s += "<div class=\"ccl %cc\">&nbsp;&nbsp;%fn&nbsp;</div>";
      }

      s += "<div class=\"cct %cc\">&nbsp;%fs&nbsp;</div>";

      s += "<div class=\"ccr %cc\">&nbsp;";

      if ((flags & (1 << 0)) != 0) { // flagcandelete
        s += "<button title=\"Delete\" onclick=\"deletefile('%fn')\">D</button>";
      }
      if ((flags & (1 << 1)) != 0) { // flagcanrename
        s += "<button title=\"Rename\" onclick=\"renamefile('%fn')\">R</button>";
      }
      if ((flags & (1 << 2)) != 0) { // flagcanedit
        s += "<button title=\"Edit\" onclick=\"editfile('%fn')\">E</button>";
      }
      if ((flags & (1 << 3)) != 0) { // flagcanpreview
        s += "<button title=\"Preview\" onclick=\"previewfile('%fn')\">P</button> ";
      }
      if ((flags & (1 << 4)) != 0) { // flagcangzip
        s += "<button title=\"Compress\" onclick=\"compressurlfile('%fn')\">C</button> ";
      }
      if (flags != 0) {
        s += "&nbsp;";
      }

      s += "</div>";

      // Makros auflösen                  
      s = s.replaceAll("%fn", items[c + 0]);
      s = s.replaceAll("%fd", items[c + 1]);
      s = s.replaceAll("%fs", items[c + 2]);
      s = s.replaceAll("%cc", items[c + 3]);

      c += 5;
      itemhtml += s;
    }

    itemhtml += "</div><div class=\"uc\">preview</div></div>";

    elementfi.innerHTML = itemhtml;

    msgline("");
  }
}

//000000000000000000000000000
function getbootinfo() {
  msgline("Get display infos...");
  var request = new XMLHttpRequest();
  request.onprogress = function(evt) {
     msgline("Fetching display infos, "+evt.loaded + "  B received.");
  }
  request.onreadystatechange = BootAnswerProcessor;
  request.open('GET', '/b', true);
  request.send(null);
}

//000000000000000000000000000
function BootAnswerProcessor() {
  var DONE = this.DONE || 4;
  if (this.readyState === DONE) {
    var antworttrenner = String.fromCharCode(02, 01, 07);
    var res = this.responseText.split(antworttrenner);

    // console.log('Bootinfos: '+res.length);
    
    // ESPFMfGK::Backgroundcolor
    if (res[0]!="") {
      var c = document.getElementsByClassName('background');
      for (i = 0; i < c.length; i++) {
        c[i].style.backgroundColor = res[0];
      }
    }
    // ESPFMfGK::ExtraHTMLfoot
    if (res[1]!="") {
      var d = document.getElementById("foot");
      d.innerHTML = res[1];
    }

    // und nun kann die Dateiliste geholt werden
    getfileinsert();
  }
}

//000000000000000000000000000
function fsselectonchange() {
  getfileinsert();
}

//000000000000000000000000000
function downloadfile(filename) {
  window.location.href = "/job?fs=" + getFileSystemIndex() + "&job=download&fn=" + filename;
}

//000000000000000000000000000
function deletefile(filename) {
  if (confirm("Really delete " + filename)) {
    msgline("Please wait. Delete in progress...");
    executecommand("job=del&fn=" + filename);
  }
}

//000000000000000000000000000
function renamefile(filename) {
  var newname = prompt("new name for " + filename, filename);
  if (newname != null) {
    msgline("Please wait. Rename in progress...");
    executecommand("job=ren&fn=" + filename + "&new=" + newname);
  }
}

var editxhr;

//000000000000000000000000000
function editfile(filename) {
  msgline("Please wait. Creating editor...");

  editxhr = new XMLHttpRequest();
  editxhr.onreadystatechange = function() {
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
      elementfi.innerHTML = editxhr.responseText;
      elemento3.innerHTML = "Edit " + filename;
      msgline("");
    }
  };
  editxhr.open('GET', "/job?fs=" + getFileSystemIndex() + "&job=edit&fn=" + filename, true);
  editxhr.send(null);
}

//000000000000000000000000000
function sved(filename) {
  var content = document.getElementById('tect').value;
  // utf-8
  content = unescape(encodeURIComponent(content));

  var xhr = new XMLHttpRequest();

  xhr.open('POST', '/r?fs=' + getFileSystemIndex() + '&fn=' + filename);

  var boundary = '-----whatever';
  xhr.setRequestHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  var body = "" +
    '--' + boundary + '\r\n' +
    'Content-Disposition: form-data; name="uploadfile"; filename="' + filename + '"' + '\r\n' +
    'Content-Type: text/plain' + '\r\n' +
    '' + '\r\n' +
    content + '\r\n' +
    '--' + boundary + '--\r\n' + // \r\n fixes upload delay in ESP8266WebServer
    '';

  // ajax does not do xhr.setRequestHeader("Content-length", body.length);

  xhr.onreadystatechange = function() {
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
      getfileinsert();
    }
  }

  xhr.send(body);
}

//000000000000000000000000000
function abed() {
  getfileinsert();
}

var uploaddone = true; // hlpr for multiple file uploads
var lastpercentComplete = -1;

//000000000000000000000000000
function uploadFile(file, islast) {
  uploaddone = false;
  var xhr = new XMLHttpRequest();
  lastpercentComplete = -1;
  xhr.upload.onprogress = function(evt) {
    if (evt.lengthComputable) {
      var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
      if (lastpercentComplete != percentComplete) {
        lastpercentComplete = percentComplete;
        msgline("Please wait. Upload progress "+ percentComplete + "%");
        // console.log('Progress: '+percentComplete);
      }
    } 
  }
  xhr.onreadystatechange = function() {
    // console.log(xhr.status);
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
      if (islast) {
        getfileinsert();
        console.log('last file');
      }
      uploaddone = true;
    }
  };
  xhr.open('POST', '/r?fs=' + getFileSystemIndex() + '&fn=' + file.name);
  var formdata = new FormData();
  formdata.append('uploadfile', file);
  // not sure why, but with that the upload to esp32 is stable.
  formdata.append('dummy', 'dummy');
  xhr.send(formdata);
}

var globaldropfilelisthlpr = null; // read-only-list, no shift()
var transferitem = 0;
var uploadFileProzessorhndlr = null;

//000000000000000000000000000
function uploadFileProzessor() {
  if (uploaddone) {
    if (transferitem == globaldropfilelisthlpr.length) {
      clearInterval(uploadFileProzessorhndlr);
    } else {
      var file = globaldropfilelisthlpr[transferitem];
      msgline("Please wait. Transferring file " + file.name + "...");
      console.log('process file ' + file.name);
      transferitem++;
      uploadFile(file, transferitem == globaldropfilelisthlpr.length);
    }
  }
}

//000000000000000000000000000
function dropHandler(ev) {
  console.log('File(s) dropped');

  globaldropfilelisthlpr = ev.dataTransfer;
  transferitem = 0;

  msgline("Please wait. Transferring file...");

  // Prevent default behavior (Prevent file from being opened)
  ev.preventDefault();

  if (ev.dataTransfer.items) {
    var data = ev.dataTransfer;
    globaldropfilelisthlpr = data.files;
    uploadFileProzessorhndlr = setInterval(uploadFileProzessor, 1000);
    console.log('Init upload list.');
  } else {
    // Use DataTransfer interface to access the file(s)
    for (var i = 0; i < ev.dataTransfer.files.length; i++) {
      console.log('.2. file[' + i + '].name = ' + ev.dataTransfer.files[i].name);
    }
  }
}

//000000000000000000000000000
function dragOverHandler(ev) {
  console.log('File(s) in drop zone');

  // Prevent default behavior (Prevent file from being opened)
  ev.preventDefault();
}

//000000000000000000000000000
function msgline(msg) {
  elementmsg.innerHTML = msg;
}

//000000000000000000000000000
function downloadall() {
  msgline("Sending all files in one zip.");
  window.location.href = "/c?za=all";
  msgline("");
}

//000000000000000000000000000
function boot() {
  // Does lookup need time?  
  elemento2 = document.getElementById("o2");
  elemento3 = document.getElementById('o3');
  elementmsg = document.getElementById('msg');
  elementfi = document.getElementById("fi");

  getbootinfo();  
}

//->
window.onload = boot;
  )==x==";


//*****************************************************************************************************
static const char ESPFMfGKWpcss[] PROGMEM = R"==g==(

.background {
  background-color: black;
}

div {
  margin: 1px;
  padding: 0px;
  font-family: 'Segoe UI', Verdana, sans-serif;
}

#gc {
  display: grid;
  grid-template-columns: 80px 25% auto 30px;
  grid-template-rows: 25px 30px auto 30px 20px;
  grid-template-areas: "o1 o2 o3 o4" "m1 c c c" "m2 c c c" "m3 c c c" "u1 u2 u3 u4";
}

.o1 {
  grid-area: o1;
  background-color: #9999CC;
  border-top-left-radius: 20px;
  margin-bottom: 0px;
}

.o2 {
  grid-area: o2;
  background-color: #9999FF;
  margin-bottom: 0px;
}

.o2 select  {
    border: 0px;
    color: white;
    font-weight: bold;
    background-color: transparent;
}

.o2 option {
    border: 0px;
    color: black;
    background-color: white;
}

.o3 {
  grid-area: o3;
  background-color: #CC99CC;
  margin-bottom: 0px;
  white-space: nowrap;
}

.o4 {
  grid-area: o4;
  background-color: #CC6699;
  border-radius: 0 10px 10px 0;
  margin-bottom: 0px;
}

.m1 {
  grid-area: m1;
  margin-top: 0px;
  background-color: #9999CC;
  display: grid;
  grid-template-columns: 60px 20px;
  grid-template-rows: 20px;
  grid-template-areas: "s11 s12";  
}

.s12 {
  margin: 0px;
  background-color: #9999CC;
}

.s13 {
  margin: 0px;
  border-top-left-radius: 20px;
  height: 30px;
}

.m2 {
  display: flex;
  justify-content: center; 
  align-items: center;
  grid-area: m2;
  background-color: #CC6699;
  width: 60px;
}

.m3 {
  grid-area: m3;
  margin-bottom: 0px;
  background-color: #9999CC;
  display: grid;
  grid-template-columns: 60px 20px;
  grid-template-rows: 20px;
  grid-template-areas: "s31 s32";  
}

.s32 {
  margin: 0px;
  background-color: #9999CC;
}

.s33 {
  margin: 0px;
  border-bottom-left-radius: 20px;
  height: 30px;
}

.u1 {
  grid-area: u1;
  background-color: #9999CC;
  border-bottom-left-radius: 20px;
  margin-top: 0px;
}

.u2 {
  grid-area: u2;
  cursor: pointer;
  background-color: #CC6666;
  margin-top: 0px;
  padding-left: 10px;
  vertical-align: middle;
  font-size: 80%;
}

.u2:hover {
  background-color: #9999FF;
  color: white;
}

.u3 {
  grid-area: u3;
  padding-left: 10px;
  background-color: #FF9966;
  font-size: 80%;
  margin-top: 0px;
}

.u4 {
  grid-area: u4;
  background-color: #FF9900;
  border-radius: 0 10px 10px 0;
  margin-top: 0px;
}

.c {
  grid-area: c;
}

#fi button {
  background-color: Transparent;
  border: 1px solid #9999FF;  
  border-radius: 1px;
  padding: 0px;
  width: 30px;
  cursor: pointer;
}

#fi button:hover {
  background-color: #9999FF;
  color: white;
}

.cc {
  width: min-content;
  margin: 10px 0px;

  display: grid; 
  grid-auto-rows: 1fr; 
  grid-template-columns: 1fr 1fr; 
  gap: 0px 0px;   
}

.gc div {
  padding: 1px;  
}

.cc .uc {
  background-color: green;
  display: none;
}

.ccg, ccu {
  height: 1.5em;
}

.ccg {
  background-color: #A5A5FF;
}

.ccu {
  background-color: #FE9A00;
}

.ccl {
  border-radius: 5px 0 0 5px;
  cursor: pointer;
}

.ccl:hover {
  border-radius: 5px 0 0 5px;
  color: white;
  cursor: pointer;
}

.ccr {
  border-radius: 0 5px 5px 0;
}

.cct {
  text-align: right;
}

.gc {
  display: grid;
  grid-template-columns: repeat(3, max-content);  
}  
  )==g==";


#endif