// inline guard. Did I mention that c/c++ is broken by design?
#ifndef ESPFMfGKWp_h
#define ESPFMfGKWp_h

// this file has been created by makeESPxWebFlMgrWp\do.cmd

//*****************************************************************************************************
static const char ESPFMfGKWpindexpage[] PROGMEM = R"==x==(
<!DOCTYPE html>
<html lang="en">

<head>
  <title>FileManager</title>
  <meta charset="utf-8" />
  <link rel="stylesheet" type="text/css" href="/fm.css">
  <script src="/fm.js"></script>
</head>

<body class="background">
  <div id="gc">
    <div class="o1">
      <svg id="wait" width="30" height="30" style="border: 1px solid #9999CC; background-color:#9999CC; ">
        <circle stroke="#CC6699" stroke-width="1" r="10" cx="15" cy="15" fill="#9999CC" />
        <circle id="dot" r="5" cx="7.5" cy="7.5" fill="#FF9966" />
        <animateTransform href="#dot" attributeName="transform" type="rotate" from="0 15 15" to="360 15 15" dur="3s" repeatCount="indefinite" />
      </svg>
    </div>
    <!-- <div class="o2" id="o2">&nbsp;</div> -->
    <div class="o2" id="o2">
      <div id="o2i1">&nbsp;</div>
      <div id="o2i2" title="Create an empty file" onclick="makeemptyfile();">&nbsp;&#xFF0B;&nbsp;</div>      
    </div>
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
    <div class="c" id="ca">
      <div id="pi">
        Path name should appear here.
      </div>
      <div>
        <div id="ti">
          Tree List should appear here.
        </div>
        <div id="fi">
          File list should appear here.
        </div>
        <div id="ei">
          Editor should appear here.
        </div>
        <div id="pv">
          Preview should appear here.
        </div>
      </div>
    </div>
  </div>
  <div id="foot"></div>
</body>

</html>

  )==x==";

static const char ESPFMfGKWpjavascript[] PROGMEM = R"==x==(

var elemento2i1 = null;
var elemento3 = null;
var elementmsg = null;
var elementfi = null;
var elementei = null;
var elementpv = null;
var elementti = null;
var elementpi = null;
var elementws = null;

var sektionstrenner = String.fromCharCode(3, 1, 2);
var antworttrenner = String.fromCharCode(2, 1, 3);
var itemtrenner = String.fromCharCode(2, 1, 4);
var bootinfotrenner = String.fromCharCode(2, 1, 7);

var foldername = "";
var windowcounter = 0;

var pathinsertintro =
    "<div id=\"pl\"><div class=\"po1\"></div><div class=\"po2\"><div></div></div><div class=\"po3\"></div><div class=\"po4\"><div>";

var pathinsertextro =
    "</div></div><div class=\"po5\"></div><div class=\"po6\"><div></div></div><div class=\"po7\"></div>" +
    "<div class=\"pu1\"></div><div class=\"pu2\"></div><div class=\"pu3\"></div><div class=\"pu4\">&nbsp;</div>" +
    "<div class=\"pu5\"></div><div class=\"pu6\"></div><div class=\"pu7\"></div></div>";

var windowhtml = "<div id=\"%i%\"><div class=\"windowtitle\"><div class=\"t\">%t%</div><div class=\"g\"></div>"+
"<div class=\"windowclose\">&nbsp;</div></div><div class=\"windowcontent\"></div>"+
"<div class=\"windowgrip\">:::</div></div>";

//000000000000000000000000000
function compressurlfile(source) {
    // dynamic post load js    
    var se = document.createElement('script');
    se.type = 'text/javascript';
    se.src = "/gzipper.js";
    document.head.appendChild(se);

    msgline("Fetching file...");
    var request = new XMLHttpRequest();
    request.onprogress = function (evt) {
        if (evt.lengthComputable) {
            var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
            if (lastpercentComplete != percentComplete) {
                lastpercentComplete = percentComplete;
                msgline("Fetching file " + percentComplete + "%...");
            }
        }
    }
    request.onreadystatechange = function () {
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
            sendback.upload.onprogress = function (evt) {
                if (evt.lengthComputable) {
                    var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
                    if (lastpercentComplete != percentComplete) {
                        lastpercentComplete = percentComplete;
                        msgline("Sending compressed file " + percentComplete + "%...");
                    }
                }
            }
            sendback.onreadystatechange = function () {
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
    request.open('GET', source + "?fs=" + getFileSystemIndex(), true);
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
    xhr.onreadystatechange = function () {
        var DONE = this.DONE || 4;
        if (this.readyState === DONE) {
            getfileinsert();
        }
    };
    xhr.open('GET', '/job?fs=' + getFileSystemIndex() + "&" + command, true);
    xhr.send(null);
}

//000000000000000000000000000
function showfolder(level, folder) {
    foldername = folder;
    getfileinsert();
}

//000000000000000000000000000
function progressfunc(evt) {
    /*        
          var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
          if (lastpercentComplete != percentComplete) {
            lastpercentComplete = percentComplete;
    */
    msgline("Fetching data: " + evt.loaded + "  B");
}

//000000000000000000000000000
function getfileinsert() {
    var param = getFileSystemIndex();
    var cb = document.getElementById("treeview");
    var wantstree = false;
    if (cb) {
        wantstree = cb.checked;
    } else {
        foldername = "";
    }

    msgline("Fetching files infos...");
    var request = new XMLHttpRequest();
    request.onprogress = progressfunc;
    request.onreadystatechange = AnswerProcessor;
    request.open('GET', '/i?fs=' + param + '&t=' + wantstree + '&pn=' + foldername, true);
    request.send(null);
    waitspinner(true);
}

//000000000000000000000000000
function hidepathtree() {
    elementpi.innerHTML = "";
    elementti.innerHTML = "";
    elementpi.style.visibility = "collapse";
    elementti.style.visibility = "collapse";
}

//000000000000000000000000000
// Handle the getfileinsert answer
function AnswerProcessor() {
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
        var t = this.responseText;
        var sections = this.responseText.split(sektionstrenner);

        hidepathtree();

        // Sektion 0: die Ordnerliste 
        var fullpath = "";
        var items = sections[0].split(itemtrenner);
        if ((items.length > 0) && (items[0] != "")) {
            haspathtop = true;
            fullpath = "/";
            var c = 0;
            var p = "";
            var s = "<div class=\"fnecc\"><div class=\"fnegc\">";
            while (c < items.length - 1) {
                var entry = items[c].split(":");
                var level = parseInt(entry[0]);
                var pathitems = entry[1].split("/");
                var skip = false;
                // Sonderregeln
                if (level == -1) { // level up folder
                    p += "<span class=\"fneba\" onclick=\"showfolder(" + (-1) + ",'/');\">\u25B2</span>&nbsp;&nbsp;/";
                    skip = true;
                } else
                    if (level == -2) { // current folder name
                        fullpath = entry[1]; 
                        var fp = "";
                        for (var k=1;k<pathitems.length;k++) {
                           fp += "/"+pathitems[k];
                           p += "<span class=\"fneba\" onclick=\"showfolder(" + (-1) + ",'" + fp + "');\">"+ pathitems[k] +"</span>/";
                        }
                        skip = true;
                    } else {
                        s += "<div class=\"fne\" onclick=\"showfolder(" + (level + 1) + ",'" + entry[1] + "');\">";
                    }

                if (!skip) {
                    var level = parseInt(entry[0]);
                    for (let i = 0; i < level; i++) {
                        s += "&nbsp;&nbsp;";
                    }
                    s += "<span class=\"fnes fnesl\">" + pathitems[pathitems.length - 1] + "</span></div>";
                }
                c++;
            }
            s += "</div></div>";
            elementti.innerHTML = s;
            if (p != "") {
                elementpi.innerHTML = pathinsertintro + p + pathinsertextro;
                elementpi.style.visibility = "visible";
            } else {
                elementpi.style.visibility = "collapse";
            }
            elementti.style.visibility = "visible";
        }

        // Sektion 1: die Dateiliste
        var res = sections[1].split(antworttrenner);

        elemento3.innerHTML = res[1];
        elemento2i1.innerHTML = res[2];

        var items = res[0].split(itemtrenner);
        var itemhtml = "<div class=\"cc\"><div class=\"gc\">";

        var c = 0;
        while (c < items.length - 3) {
            var s = "";

            // Flags, sync with class ESPxWebFlMgr2
            var flags = parseInt(items[c + 4]);
            // s += flags.toString(2);

            if ((flags & (1 << 5)) != 0) { // flagcandownload
                s += "<div class=\"ccl %cc\" onclick=\"downloadfile(\'%fn\')\">" +
                    "<span title=\"Download\">" +
                    "&nbsp;&nbsp;%fd&nbsp;" +
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
                s += "<button title=\"Rename/Move\nEven to a difference device.\" onclick=\"renamefile('%fn')\">R</button>";
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

            // Makros aufloesen
            // Displayname ist umstaendlich, weil die Datenhaltung etwas umstaendlich ist
            if (fullpath!="") {
              var fdp = "";
              if (fullpath=="/") {
                fdp = items[c + 1].substring(1);
              } else {
                fdp = items[c + 1].substring(fullpath.length+1);
              }
              s = s.replaceAll("%fd", fdp);
            } else {    
              s = s.replaceAll("%fd", items[c + 1]);
            }
            s = s.replaceAll("%fn", items[c + 0]);
            s = s.replaceAll("%fs", items[c + 2]);
            s = s.replaceAll("%cc", items[c + 3]);

            c += 5;
            itemhtml += s;
        }

        itemhtml += "</div><div class=\"uc\">preview</div></div>";

        elementfi.innerHTML = itemhtml;

        msgline("");
        waitspinner(false);
    }
}

//000000000000000000000000000
function getbootinfo() {
    msgline("Get display infos...");
    var request = new XMLHttpRequest();
    request.onprogress = progressfunc;
    request.onreadystatechange = BootAnswerProcessor;
    request.open('GET', '/b', true);
    request.send(null);
}

//000000000000000000000000000
function BootAnswerProcessor() {
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
        var res = this.responseText.split(bootinfotrenner);

        // console.log('Bootinfos: '+res.length);

        // ESPxWebFlMgr2::Backgroundcolor
        if ((res.length>=1) && (res[0] != "")) {
            var c = document.getElementsByClassName('background');
            for (i = 0; i < c.length; i++) {
                c[i].style.backgroundColor = res[0];
            }
        }
        // ESPxWebFlMgr2::ExtraHTMLfoot
        if ((res.length>=2) && (res[1] != "")) {
            var d = document.getElementById("foot");
            d.innerHTML = res[1];
        }

        // Seitentitle
        if ((res.length>=3) && (res[2] != "")) {
           document.title = res[2];
        }

        // und nun kann die Dateiliste geholt werden
        getfileinsert();
    }
}

//000000000000000000000000000
function fsselectonchange() {
    foldername = "";
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
function makeemptyfile(filename) {
    msgline("Please wait. Create new empty file...");
    executecommand("job=createnew&fn=" + foldername + "/newfile");
}

//000000000000000000000000000
function renamefile(filename) {
    var newname = prompt("new name for " + filename, filename);
    if (newname != null) {
        msgline("Please wait. Rename in progress...");
        executecommand("job=ren&fn=" + filename + "&new=" + newname);
    }
}

//000000000000000000000000000
function previewfile(filename) {
    msgline("Please wait. Creating preview...");

    var previewxhr = new XMLHttpRequest();
    previewxhr.responseType = "blob";
    previewxhr.onreadystatechange = function () {
        var DONE = this.DONE || 4;
        if (this.readyState === DONE) {
            var newwin = windowhtml;

            var winid = "win"+windowcounter;
            newwin = newwin.replaceAll("%i%", "win"+windowcounter);
            newwin = newwin.replaceAll("%t%", filename);
            var elem = document.createRange().createContextualFragment(newwin);
            document.body.appendChild(elem);

            // console.log(previewxhr.getResponseHeader('content-type'));
            // alles furchtbar umständlich, weil bilder nur als BLOB funktionieren und Text daher wieder aus dem Blob gelesen werden muss... 

            var content = document.querySelector("#"+winid+" .windowcontent");
            var dragger = document.querySelector("#"+winid+" .windowtitle");
            var winid = '#'+"win"+windowcounter;
            var node = document.querySelector(winid);

            if (previewxhr.getResponseHeader('content-type').startsWith("image/")) {
              var image = new Image();
              image.src = URL.createObjectURL(previewxhr.response);
              content.appendChild(image);
              makeDraggable(node);
            } else {
              content.style.whiteSpace = "pre";
              const reader = new FileReader();
              reader.addEventListener('loadend', (e) => {
                content.textContent = e.srcElement.result;
                // Warten, bis Content vollständig ASYNC!
                makeDraggable(node);
              });
              reader.readAsText(previewxhr.response);
            }             

            windowcounter++;
            msgline("");
            waitspinner(false);
        }
    };
    previewxhr.onprogress = progressfunc;
    previewxhr.open('GET', "/job?fs=" + getFileSystemIndex() + "&job=preview&fn=" + filename, true);
    previewxhr.send(null);
    waitspinner(true);
}

//000000000000000000000000000
function editfile(filename) {
    msgline("Please wait. Creating editor...");

    var editxhr = new XMLHttpRequest();
    editxhr.onreadystatechange = function () {
        var DONE = this.DONE || 4;
        if (this.readyState === DONE) {
            hidepathtree();
            elementfi.innerHTML = "";
            elementfi.style.visibility = "collapse";
            elementei.innerHTML = this.responseText;
            elementei.style.display = "block";
            elemento3.innerHTML = "Edit " + filename;

            var elem = document.getElementById("tect");
            elem.style.height = (window.innerHeight-120)+"px";
            elem.style.width = (window.innerWidth-150)+"px";

            msgline("");
            waitspinner(false);
        }
    };
    editxhr.onprogress = progressfunc;
    editxhr.open('GET', "/job?fs=" + getFileSystemIndex() + "&job=edit&fn=" + filename, true);
    editxhr.send(null);
    waitspinner(true);
}

//000000000000000000000000000
function sved(filename) {
    var content = document.getElementById('tect').value;
    // utf-8, this does not work.
    // content = unescape(encodeURIComponent(content));

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

    xhr.onreadystatechange = function () {
        var DONE = this.DONE || 4;
        if (this.readyState === DONE) {
            elementei.innerHTML = "";
            elementei.style.display = "none";
            elementfi.style.visibility = "visible";
            getfileinsert();
        }
    }
    xhr.onprogress = progressfunc;

    xhr.send(body);
}

//000000000000000000000000000
function abed() {
    elementei.innerHTML = "";
    elementei.style.display = "none";
    elementfi.style.visibility = "visible";
    getfileinsert();
}

var uploaddone = true; // hlpr for multiple file uploads
var lastpercentComplete = -1;

//000000000000000000000000000
function uploadFile(file, islast) {
    uploaddone = false;
    var xhr = new XMLHttpRequest();
    lastpercentComplete = -1;
    xhr.upload.onprogress = progressfunc;
    xhr.onreadystatechange = function () {
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
    xhr.open('POST', '/r?fs=' + getFileSystemIndex() + '&fn=' + foldername + "/" + file.name);
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
    waitspinner(true);
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
function waitspinner(ison) {
    if (ison) {
        elementws.unpauseAnimations();
        elementws.style.visibility = "visible";
    } else {
        elementws.pauseAnimations();
        elementws.style.visibility = "collapse";
    }
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
function makeDraggable(box) {
    let cX = 0, cY = 0, pX = 0, pY = 0;

    box.setAttribute("floater", "true");
    box.classList.add("windowstyle");
    let content = box.querySelector('.windowcontent');
    if (content) {
      if (content.clientHeight>window.innerHeight) {
        content.style.height=window.innerHeight/2+"px";
      }
      if (content.clientWidth>window.innerWidth) {
        content.style.width=window.innerWidth/2+"px";
      }
    }

    let title = box.querySelector('.windowtitle');
    if (title) {
        title.addEventListener('mousedown', startDrag);
    }
    let resizer = box.querySelector('.windowgrip');
    if (resizer) {
        resizer.addEventListener('mousedown', startResize);
    }
    let closer = box.querySelector('.windowclose');
    if (closer) {
        closer.addEventListener('click', closewindow);
    }
    bringbox2front();

    // Closing
    function closewindow() {
        closer.removeEventListener('click', closewindow);
        resizer.removeEventListener('mousedown', startResize);
        title.removeEventListener('mousedown', startDrag);
        box.innerHTML = "";
        box.style.display = "none";
    }

    // Stacking
    function bringbox2front() {
        let all = document.querySelectorAll('[floater]');
        let maxz = -1;

        for (var i = 0; i < all.length; i++) {
            if (all[i] != box) {
                let zi = parseInt(window.getComputedStyle(all[i]).zIndex);
                if ((zi > maxz)) {
                    maxz = zi;
                }
            }
        }
        if (window.getComputedStyle(box).zIndex <= maxz) {
            box.style.zIndex = maxz + 1;
        }
    }

    // Resizeing
    function startDrag(e) {
        bringbox2front();
        e.preventDefault();
        pX = e.clientX;
        pY = e.clientY;
        document.addEventListener('mouseup', endDrag);
        document.addEventListener('mousemove', drag);
    }

    function drag(e) {
        e.preventDefault();
        cX = pX - e.clientX;
        cY = pY - e.clientY;
        pX = e.clientX;
        pY = e.clientY;
        box.style.top = (box.offsetTop - cY) + 'px';
        box.style.left = (box.offsetLeft - cX) + 'px';
    }

    function endDrag() {
        document.removeEventListener('mouseup', endDrag);
        document.removeEventListener('mousemove', drag);
    }

    // Resize
    function startResize(e) {
        e.preventDefault();
        pX = e.clientX;
        pY = e.clientY;
        document.addEventListener('mouseup', endResize);
        document.addEventListener('mousemove', resize);
    }

    function resize(e) {
        e.preventDefault();
        box.style.width = e.pageX - box.getBoundingClientRect().left + 'px';
        box.style.height = e.pageY - box.getBoundingClientRect().top + 'px';
        if (content) {
          content.style.width = e.pageX - content.getBoundingClientRect().left + 'px';
          content.style.height = e.pageY - content.getBoundingClientRect().top + 'px';
        }
    }

    function endResize() {
        document.removeEventListener('mouseup', endResize);
        document.removeEventListener('mousemove', resize);
    }
}

//000000000000000000000000000
function boot() {
    // Does lookup need time?  
    elemento2i1 = document.getElementById("o2i1");
    elemento3 = document.getElementById('o3');
    elementmsg = document.getElementById('msg');
    elementfi = document.getElementById("fi");
    elementei = document.getElementById("ei");
    elementpv = document.getElementById("pv");
    elementti = document.getElementById("ti");
    elementpi = document.getElementById("pi");
    elementws = document.getElementById("wait");

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
  padding: 0;
  font-family: 'Segoe UI', Verdana, sans-serif;
}

#gc {
  display: grid;
  grid-template-columns: 80px 25% auto 30px;
  grid-template-rows: 25px 30px auto 30px 20px;
  grid-template-areas: "o1 o2 o3 o4""m1 c c c""m2 c c c""m3 c c c""u1 u2 u3 u4";
}

#wait {
  position: relative;
  top: 10px;
  left: 15px;
  visibility: collapse;
}

#ca, #ta, #ti, #pi, #fi {
  margin: 0;
  padding: 0;
}

#ca {}

#ti {
  float: left;
}

#fi {}

#ei, #pv {
  display: none;
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
  white-space: nowrap;
  display: grid; 
  grid-auto-rows: 1fr; 
  grid-template-columns: 1fr 20px; 
  gap: 0px 0px; 
}

#o2i2 {
  background-color: #BBB2FF;
  margin: 0;
  padding: 0;
  cursor: default;
  color: white;
}

#o2i2:hover {
  background-color: #D36669;
  color: black;
}

.o2 select {
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

.o2 label {
  color: white;
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

.fnecc {
  width: min-content;
  margin: 0 1px;
  padding-top: 2px;
  display: grid;
  grid-auto-rows: 1fr;
  grid-template-columns: 1fr 1fr;
  gap: 0px 0px;
}

.fnegc {
  margin: 0;
  display: grid;
  grid-template-columns: repeat(1, max-content);
}

.fne {
  margin: 1px 0px 2px 0px;
  background-color: white;
}

.fnegc div {
  padding: 1px;
}

.fnes {
  padding: 0px 5px 0px 5px;
  cursor: pointer;
  border-radius: 5px;
}

.fnesl {
  border: 1px solid #EDC315;
  background-color: #EDC315;
  color: black;
}

.fnesl:hover {
  border: 1px solid #EDC315;
  background-color: #EDC315;
  color: white;
}

.fnesa {
  border: 1px solid #F8FBDB;
  background-color: #F8FBDB;
  color: #white;
}

/* back button */
.fneba {
  background-color: #EDC315;
  border: 3px solid #EDC315;
  border-radius: 4px;
  padding: 0px 5px 0px 5px;
  margin: 0;
  font-size: 0.7em;
  cursor: pointer;
}

.fneba:hover {
  color: white;
  background-color: #9999FF;
  border: 3px solid #9999FF;
  border-radius: 4px;
}

// EditorInsert
#ei button {
  white-space: nowrap; 
  background-color: #F8FBDB;
  border: 3px solid #F8FBDB;
  border-radius: 4px;
  padding: 5px 3px 5px 3px;
  margin-bottom: 0.7em;
  font-weight: bold;
  cursor: pointer;
}

#ei button:hover {
  background-color: #9999FF;
  border-radius: 4px;
  border: 3px solid #F8FBDB;
  color: white;
}


/* pfad tabelle */
#pl {
  display: inline-grid; 
  grid-template-columns: auto 20px 40px 1fr 40px 20px auto; 
  grid-template-rows: auto auto; 
  gap: 0px 0px; 
}

#pl div {
  margin: 0;
  padding: 0;
}

#pl .pu1 {
  width: 1em;
  border-radius: 0 0 0 1em;
  background-color: #FF9966;
  height: 100%;
}

#pl .pu7 {
  width: 1em;
  border-radius: 0 0 1em 0;
  background-color: #FF9966;
  height: 100%;
}

#pl .po2, #pl .po6 {
  background-color: #FF9966;
}
#pl .po2 div {
  width: 1em;
  border-radius: 0 0 0 0.7em;
  background-color: white;
  height: 100%;
  width: 100%;
}

#pl .po6 div {
  width: 1em;
  border-radius: 0 0 0.7em 0;
  background-color: white;
  height: 100%;
  width: 100%;
}

#pl .po1, #pl .po7, #pl .pu2, #pl .pu6  {
  background-color: #FF9966;
}
#pl .pu3, #pl .pu5  {
  background-color: #CC6666;
}
#pl .pu4  {
  background-color: #FFCC99;
  font-size: 0.7em;
}
#pl .po4  {
  text-align: center;
  height: 1.7em;
}

#pl .po4 > div {
  margin-top: 0.1em;
}

/* Windows */
.windowstyle {
    margin: 0;
    padding: 0;
    position: absolute;
    z-index: 9;
    background-color: transparent;
}

.windowcontent {
    overflow:scroll;
    overflow-x:hidden;
    position: relative;
    margin: 0;
    padding: 0;
    background-color: #f1f1f1;
    border-left: 2px solid #D8A570;
}

.windowtitle {
    display: grid; 
    margin: 0;
    grid-auto-rows: 1fr; 
    grid-template-columns: 1fr 2px 20px; 
    gap: 0px 0px; 
}    
 
.windowtitle .g {
    background-color: white;
    margin:0;
}

.windowtitle .t {
    padding-right: 10px;
    padding-left: 10px;
    margin:0;
    cursor: move;
    z-index: 12;
    background-color: #D8A570;
    border-top-left-radius: 10px;
    color: #fff;
}

.windowclose {
    cursor: default;
    padding: 0;
    margin: 0;
    font-size: 0.1em;
    border-top-right-radius: 10px;
    border-bottom-right-radius: 10px;
    background-color: #B67CD0;
}

.windowclose:hover {
    background-color: #959FFE;
}


.windowgrip {
    z-index: 12;
    margin: 0;
    padding: 0;
    float: right;
    position: absolute;
    bottom: 0;
    right: 10px;
    width: 1em;
    cursor: nwse-resize;
}

  )==g==";


#endif
