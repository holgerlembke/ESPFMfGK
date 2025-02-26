"use strict";

// slowly moving from "var" to "let"

let elemento2i1 = null;
let elemento3 = null;
let elementmsg = null;
let elementfi = null;
let elementei = null;
let elementpv = null;
let elementti = null;
let elementpi = null;
let elementws = null;
let elementwinform = null;

// globale Dialogroutinen
let dialog = null;
let dialogOKCall = null;

const sektionstrenner = String.fromCharCode(3, 1, 2);
const antworttrenner = String.fromCharCode(2, 1, 3);
const itemtrenner = String.fromCharCode(2, 1, 4);
const bootinfotrenner = String.fromCharCode(2, 1, 7);

let foldername = "";
let windowcounter = 0;
let filesysteminfos = "";

const pathinsertintro =
    "<div id=\"pl\"><div class=\"po1\"></div><div class=\"po2\"><div></div></div><div class=\"po3\"></div><div class=\"po4\"><div>";

const pathinsertextro =
    "</div></div><div class=\"po5\"></div><div class=\"po6\"><div></div></div><div class=\"po7\"></div>" +
    "<div class=\"pu1\"></div><div class=\"pu2\"></div><div class=\"pu3\"></div><div class=\"pu4\">&nbsp;</div>" +
    "<div class=\"pu5\"></div><div class=\"pu6\"></div><div class=\"pu7\"></div></div>";

const folderdownloadinsert =
    "<label><input type=\"radio\" name=\"a\" value=\"1\"> all files on device</label><br>" +
    "<label><input type=\"radio\" name=\"a\" value=\"2\"> this folder</label><br>" +
    "<label><input type=\"radio\" name=\"a\" value=\"3\"> this folder and subfolders</label><br>";

const filerenameinsert =
    "New name:<br><input id=\"newname\" length=\"200\"><p><input type=\"hidden\" id=\"oldname\">" +
    "Changing the path will move the file. Start with file system number to move to other device.<br>" +
    "Active filesystems are: %f%.<br>" +
    "Example: 1:/path/path/filename.extention</p>";

const filedeleteinsert =
    "Delete file <i>%f%</i>?<br><input type=\"hidden\" id=\"filename\"><p><br>Deleting files is final.</p>";

const windowhtml = "<div id=\"%i%\"><div class=\"windowtitle\"><div class=\"t\">%t%</div>" +
    "<div class=\"tsi\"><div class=\"ts\">Save</div></div>" +
    "<div class=\"g\"></div>" +
    "<div class=\"windowclose\">&nbsp;</div></div><div class=\"windowcontent\"></div>" +
    "<div class=\"windowgrip\">:::</div></div>";


// Callbacks für die HTMLIncludes
let callbackFileinsert = [];


//------------------------------------------------------------------------------------------------------------
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
                msgline("Fetching file " + percentComplete + "%...");
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
                        msgline("Sending compressed file " + percentComplete + "%...");
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
    request.open('GET', source + "?fs=" + getFileSystemIndex(), true);
    request.send(null);
}

//------------------------------------------------------------------------------------------------------------
function getFileSystemIndex() {
    var selectinput = document.getElementById('memory');
    if (selectinput == null) {
        console.log('Hint: get default file system index');
        return -1;
    } else {
        return selectinput.selectedIndex;
    }
}

//------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------
function showfolder(level, folder) {
    foldername = folder;
    getfileinsert();
}

//------------------------------------------------------------------------------------------------------------
function progressfunc(evt) {
    /*        
          var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
          if (lastpercentComplete != percentComplete) {
            lastpercentComplete = percentComplete;
    */
    msgline("Fetching data: " + evt.loaded + "  B");
}

//------------------------------------------------------------------------------------------------------------
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
            let haspathtop = true;
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
                    for (var k = 1; k < pathitems.length; k++) {
                        fp += "/" + pathitems[k];
                        p += "<span class=\"fneba\" onclick=\"showfolder(" + (-1) + ",'" + fp + "');\">" + pathitems[k] + "</span>/";
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
            if (fullpath != "") {
                var fdp = "";
                if (fullpath == "/") {
                    fdp = items[c + 1].substring(1);
                } else {
                    fdp = items[c + 1].substring(fullpath.length + 1);
                }
                s = s.replaceAll("%fd", fdp);
            } else {
                s = s.replaceAll("%fd", items[c + 1]);
            }

            // Filedate & Filesize zusammenbauen
            let fds = "";
            if (items.length >= 6) {
              let fdi = parseInt(items[c + 5]);
              switch (fdi % 10) {
                case 0 : {
                  fds = items[c + 2];
                  break;
                }
                case 1 : { // title
                  let filedate = new Date(fdi*100); 
                  fds = "<span title='" + filedate.toLocaleString() + "'>" + items[c + 2] + "</spnn>";
                  break;
                }
                case 2 : { // text
                  let filedate = new Date(fdi*100); 
                  fds = items[c + 2] + "&nbsp" + filedate.toLocaleString();
                  break;
                }
              }
            } else {
              fds = items[c + 2];
            }

            s = s.replaceAll("%fn", items[c + 0]);
            s = s.replaceAll("%fs", fds);
            s = s.replaceAll("%cc", items[c + 3]);
            
            // weiterzählen
            c += 6;
            itemhtml += s;
        }

        itemhtml += "</div><div class=\"uc\">preview</div></div>";

        elementfi.innerHTML = itemhtml;

        msgline("");
        waitspinner(false);

        for (let i = 0; i < callbackFileinsert.length; i++) {
          callbackFileinsert[i]();
        }
    }
}

//------------------------------------------------------------------------------------------------------------
function getbootinfo() {
    msgline("Get display infos...");
    var request = new XMLHttpRequest();
    request.onprogress = progressfunc;
    request.onreadystatechange = BootAnswerProcessor;
    request.open('GET', '/b', true);
    request.send(null);
}

//------------------------------------------------------------------------------------------------------------
function BootAnswerProcessor() {
    var DONE = this.DONE || 4;
    if (this.readyState === DONE) {
        var res = this.responseText.split(bootinfotrenner);

        // console.log('Bootinfos: '+res.length);

        // ESPxWebFlMgr2::Backgroundcolor
        if ((res.length >= 1) && (res[0] != "")) {
            var c = document.getElementsByClassName('background');
            for (let i = 0; i < c.length; i++) {
                c[i].style.backgroundColor = res[0];
            }
        }
        // ESPxWebFlMgr2::ExtraHTMLfoot
        if ((res.length >= 2) && (res[1] != "")) {
            var d = document.getElementById("foot");
            d.innerHTML = res[1];
        }

        // Seitentitle
        if ((res.length >= 3) && (res[2] != "")) {
            document.title = res[2];
        }

        // Filesysteminfo
        if ((res.length >= 4) && (res[3] != "")) {
            filesysteminfos = res[3];
        }

        if ((res.length >= 5) && (res[4] != "")) {
            LoadHtmlIncludes(res[4]);
        }

        // und nun kann die Dateiliste geholt werden
        getfileinsert();
    }
}

let linksverschiebung = 0;

//------------------------------------------------------------------------------------------------------------
function linksverschieber(node) {
  // Fensterposition
  node.style.top = 100 + linksverschiebung + "px";
  node.style.left = 100 + linksverschiebung + "px";
  linksverschiebung += 20;
}

//------------------------------------------------------------------------------------------------------------
function LoadHtmlIncludesProcessor() {
    const DONE = this.DONE || 4;
    if (this.readyState === DONE) {
        let res = this.responseText;

        // First comment has to be <!--XXXXX--> with xxxx the unique windows id
        const st = res.search("<!--");
        const en = res.search("-->");
        if ((st>=0) && (en>st)) {
            let id = res.substring(st+4,en).trim();

            let elem = document.createRange().createContextualFragment(res);
            document.body.appendChild(elem);
            // alles async, also warten, bis browser soweit ist.  
            let script = document.getElementById(id+".scr");
            script.addEventListener('load', function() {
                let node = document.getElementById(id);
                if (node) {
                    makeDraggable(node);
                }
                linksverschieber(node);
                // aufruf der startfunktion
                window[id]();
            });
        }
    }
}

//------------------------------------------------------------------------------------------------------------
function LoadHtmlIncludes(includelist) {
    waitspinner(true);

    let includes = includelist.split(";");

    includes.forEach(function (incl) {
        let includexhr = new XMLHttpRequest();
        includexhr.onreadystatechange = LoadHtmlIncludesProcessor;

        includexhr.open('GET', incl, true);
        includexhr.send(null);
    });
}

//------------------------------------------------------------------------------------------------------------
function fsselectonchange() {
    foldername = "";
    getfileinsert();
}

//------------------------------------------------------------------------------------------------------------
function downloadfile(filename) {
    window.location.href = "/job?fs=" + getFileSystemIndex() + "&job=download&fn=" + filename;
}

//------------------------------------------------------------------------------------------------------------
function makeemptyfile(filename) {
    msgline("Please wait. Create new empty file...");
    executecommand("job=createnew&fn=" + foldername + "/newfile");
}

//------------------------------------------------------------------------------------------------------------
function previewfile(filename) {
    msgline("Please wait. Creating preview...");

    var previewxhr = new XMLHttpRequest();
    previewxhr.responseType = "blob";
    previewxhr.onreadystatechange = function() {
        var DONE = this.DONE || 4;
        if (this.readyState === DONE) {
            var newwin = windowhtml;

            var winid = "win" + windowcounter;
            newwin = newwin.replaceAll("%i%", "win" + windowcounter);
            newwin = newwin.replaceAll("%t%", filename);
            var elem = document.createRange().createContextualFragment(newwin);
            document.body.appendChild(elem);

            // console.log(previewxhr.getResponseHeader('content-type'));
            // alles furchtbar umständlich, weil bilder nur als BLOB funktionieren und Text daher wieder aus dem Blob gelesen werden muss... 

            var content = document.querySelector("#" + winid + " .windowcontent");
            // var dragger = document.querySelector("#" + winid + " .windowtitle");
            var winid = '#' + "win" + windowcounter;
            let node = document.querySelector(winid);
            linksverschieber(node); 

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

const zeilennummernopenstyle = "0px 5px";

//------------------------------------------------------------------------------------------------------------
function showzeilennummernimeditor(event) {
  let winid = event.target.getAttribute("winid");
  let znr = document.querySelector(winid + " .line-numbers");
  if (znr.style.padding == zeilennummernopenstyle) {
    znr.style.padding = "0";
    znr.style.display = "none";
  } else {
    znr.style.padding = zeilennummernopenstyle;
    znr.style.display = "inline-block";
    //znr.style.position = "absolute";

    let textareahtml = document.querySelector(winid + " textarea");
    zeilennummernreintun(textareahtml);
  }
}

//------------------------------------------------------------------------------------------------------------
function zeilennummernreintun(textareahtml) {
  let winid = textareahtml.getAttribute("winid");
  let znr = document.querySelector(winid + " .line-numbers");

  if (znr.style.padding == zeilennummernopenstyle) {
    let lastlln = textareahtml.getAttribute("lln");
    if (lastlln == null) {
      lastlln = 0;
    }

    lastlln = parseInt(lastlln);
    let linec = (String(textareahtml.value).match(/\n/g) || '').length + 1;

    if (lastlln!=linec) {
      textareahtml.setAttribute("lln",linec);
      let content = document.querySelector(winid + ' .windowcontent');
      znr.style.height = content.style.height;
      let s = "";
      for (let i = 1; i <= linec; i++) {
        s += "<span>" + i + "</span>" + "<br>";
      }
      znr.innerHTML = s;
    }
  }
}

//------------------------------------------------------------------------------------------------------------
function textareascrollsync(textareahtml) {
  let winid = textareahtml.getAttribute("winid");
  let znr = document.querySelector(winid + " .line-numbers");

  if (znr.style.padding == zeilennummernopenstyle) {
    znr.scrollTop = textareahtml.scrollTop;
  }
}

//------------------------------------------------------------------------------------------------------------
function zeilennummernevent(event) {
  zeilennummernreintun(event.target);
}

//------------------------------------------------------------------------------------------------------------
function textareascroll(event) {
  textareascrollsync(event.target);
}

//------------------------------------------------------------------------------------------------------------
function editfile(filename) {
    msgline("Please wait. Creating editor...");

    let editxhr = new XMLHttpRequest();
    editxhr.onreadystatechange = function() {
        let DONE = this.DONE || 4;
        if (this.readyState === DONE) {
            let newwin = windowhtml;

            let winid = "win" + windowcounter;
            newwin = newwin.replaceAll("%i%", "win" + windowcounter);
            newwin = newwin.replaceAll("%t%", filename);
            let elem = document.createRange().createContextualFragment(newwin);
            let appnode = document.body.appendChild(elem);

            winid = '#' + "win" + windowcounter;

            let content = document.querySelector(winid + " .windowcontent");
            let node = document.querySelector(winid);
            linksverschieber(node);
            document.querySelector(".windowgrip").style.display = "none";

            content.outerHTML = this.responseText; // textarea comes here

            let titlebuttons = document.querySelectorAll(winid + " .ts");
            titlebuttons.forEach((item) => {
              item.style.display = "block";
            });

            // winid für editor merken
            let editortag = document.querySelector(winid + " .idL");
            editortag.setAttribute("winid", winid);

            let zeilennummernbutton = document.querySelector(winid + " .idL");
            zeilennummernbutton.addEventListener('click', showzeilennummernimeditor);            

            let textareahtml = document.querySelector(winid + " textarea");
            textareahtml.addEventListener('keyup',zeilennummernevent);
            textareahtml.setAttribute("winid", winid);
            zeilennummernreintun(textareahtml);

            textareahtml.addEventListener("scroll",textareascroll);
            
            let savebutton = document.querySelector(winid + " .idS");
            savebutton.setAttribute("winid", winid);
//irgendwann in funktion auslagern, damit hotkeys daran geknotet werden können
            savebutton.addEventListener('click', () => {
                content = document.querySelector(winid + " textarea").value;

                let xhr = new XMLHttpRequest();

                xhr.open('POST', '/r?fs=' + getFileSystemIndex() + '&fn=' + filename);

                let boundary = '-----whatever';
                xhr.setRequestHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

                let body = "" +
                    '--' + boundary + '\r\n' +
                    'Content-Disposition: form-data; name="uploadfile"; filename="' + filename + '"' + '\r\n' +
                    'Content-Type: text/plain' + '\r\n' +
                    '' + '\r\n' +
                    content + '\r\n' +
                    '--' + boundary + '--\r\n' + // \r\n fixes upload delay in ESP8266WebServer
                    '';

                xhr.onreadystatechange = function() {
                    var DONE = this.DONE || 4;
                    if (this.readyState === DONE) {
                       // Fertig
                       getfileinsert();
                    }
                }
                xhr.onprogress = progressfunc;

                xhr.send(body);
            });

            makeDraggable(node);

            windowcounter++;
            msgline("");
            waitspinner(false);
        }
    };

    editxhr.onprogress = progressfunc;
    editxhr.open('GET', "/job?fs=" + getFileSystemIndex() + "&job=edit&fn=" + filename, true);
    editxhr.send(null);
    waitspinner(true);
}

let uploaddone = true; // hlpr for multiple file uploads
let lastpercentComplete = -1;

//------------------------------------------------------------------------------------------------------------
function uploadFile(file, islast) {
    uploaddone = false;
    var xhr = new XMLHttpRequest();
    lastpercentComplete = -1;
    xhr.upload.onprogress = progressfunc;
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
    xhr.open('POST', '/r?fs=' + getFileSystemIndex() + '&fn=' + foldername + "/" + file.name);
    var formdata = new FormData();
    formdata.append('uploadfile', file);
    // not sure why, but with that the upload to esp32 is stable.
    formdata.append('dummy', 'dummy');
    xhr.send(formdata);
}

let globaldropfilelisthlpr = null; // read-only-list, no shift()
let transferitem = 0;
let uploadFileProzessorhndlr = null;

//------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------
function dragOverHandler(ev) {
    console.log('File(s) in drop zone');

    // Prevent default behavior (Prevent file from being opened)
    ev.preventDefault();
}

//------------------------------------------------------------------------------------------------------------
function waitspinner(ison) {
    if (ison) {
        elementws.unpauseAnimations();
        elementws.style.visibility = "visible";
    } else {
        elementws.pauseAnimations();
        elementws.style.visibility = "collapse";
    }
}

//------------------------------------------------------------------------------------------------------------
function msgline(msg) {
    elementmsg.innerHTML = msg;
}

//------------------------------------------------------------------------------------------------------------
function makeDraggable(box) {
    let cX = 0,
        cY = 0,
        pX = 0,
        pY = 0;

    box.setAttribute("floater", "true");
    box.classList.add("windowstyle");
    let content = box.querySelector('.windowcontent');
    if (content) {
        if (content.clientHeight > window.innerHeight) {
            content.style.height = window.innerHeight / 2 + "px";
        } else {
            // content.style.height = content.offsetHeight + "px";
            content.style.height = "100px";
        }
        if (content.clientWidth > window.innerWidth) {
            content.style.width = window.innerWidth / 2 + "px";
        }
        content.addEventListener("click",bringbox2front);
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

    // Dragging
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
            content.style.width = (e.pageX - content.getBoundingClientRect().left) + 'px';
            content.style.height = (e.pageY - content.getBoundingClientRect().top) + 'px';
        }
    }

    function endResize() {
        document.removeEventListener('mouseup', endResize);
        document.removeEventListener('mousemove', resize);
    }
}

//------------------------------------------------------------------------------------------------------------
function deletefile(filename) {
    var fri = filedeleteinsert;
    fri = fri.replaceAll("%f%", filename);
    showdialog(deletefileinit, deletefileanalyzer, fri, "Delete file", filename);
}

//------------------------------------------------------------------------------------------------------------
function deletefileinit(formdata) {
    document.getElementById("filename").value = formdata;
}

//------------------------------------------------------------------------------------------------------------
function deletefileanalyzer(formdata) {
    let filename = document.getElementById("filename").value;

    msgline("Please wait. Delete in progress...");
    executecommand("job=del&fn=" + filename);
}

//------------------------------------------------------------------------------------------------------------
function renamefile(filename) {
    let fri = filerenameinsert;
    fri = fri.replaceAll("%f%", filesysteminfos);
    showdialog(renamefileinit, renamefileanalyzer, fri, "Rename/Move file", filename);
}

//------------------------------------------------------------------------------------------------------------
function renamefileinit(formdata) {
    document.getElementById("newname").value = formdata;
    document.getElementById("oldname").value = formdata;
}

//------------------------------------------------------------------------------------------------------------
function renamefileanalyzer(formdata) {
    let newname = document.getElementById("newname").value;
    let oldname = document.getElementById("oldname").value;

    msgline("Please wait. Rename in progress...");
    executecommand("job=ren&fn=" + oldname + "&new=" + newname);
}

//------------------------------------------------------------------------------------------------------------
function downloadmgrresanalyzer() {
    let radios = document.getElementsByTagName('input');
    let value = 0;
    for (let i = 0; i < radios.length; i++) {
        if (radios[i].type === 'radio' && radios[i].checked) {
            value = radios[i].value;
        }
    }

    if (value > 0) {
        window.location.href = "/job?fs=" + getFileSystemIndex() + "&job=dwnldll&mode=" + value + "&fn=dummy&folder=" + foldername;
    }
}

//------------------------------------------------------------------------------------------------------------
function downloadmgr() {
    showdialog(0, downloadmgrresanalyzer, folderdownloadinsert, "Download files");
}

//------------------------------------------------------------------------------------------------------------
function dialogkill() {
    document.getElementById("dok").removeEventListener("click",dialogeventlistenerHndlOK);
    document.getElementById("dcancel").removeEventListener("click",dialogeventlistenerHndlCancel);
    document.getElementById("dclose").removeEventListener("click",dialogeventlistenerHndlCancel);

    dialog = null;
    dialogOKCall = null;

    elementwinform.innerHTML = "";
}

//------------------------------------------------------------------------------------------------------------
function dialogeventlistenerHndlOK() {
  dialog.close();
  if (dialogOKCall ) {
    dialogOKCall ();
  };
  dialogkill();
}

//------------------------------------------------------------------------------------------------------------
function dialogeventlistenerHndlCancel() {
  dialog.close();
  dialogkill();
}

//------------------------------------------------------------------------------------------------------------
function showdialog(initcall, okcall, dialogitems, title, formdata) {
    dialog = document.getElementById("prompt");
    let result = document.getElementById("result");

    elementwinform.innerHTML = dialogitems;
    let html = document.getElementById("wt");
    html.innerHTML = title;

    if (initcall) {
        initcall(formdata);
    }

    document.getElementById("dok").addEventListener("click",dialogeventlistenerHndlOK);
    document.getElementById("dcancel").addEventListener("click",dialogeventlistenerHndlCancel);
    document.getElementById("dclose").addEventListener("click",dialogeventlistenerHndlCancel);
    dialogOKCall = okcall;

    // Async!
    dialog.showModal();
}

//------------------------------------------------------------------------------------------------------------
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
    elementwinform = document.getElementById("windowform");

/*
document.addEventListener('keydown', e => {
  if (e.ctrlKey && e.key === 's') {
    e.preventDefault();
    console.log('CTRL + S');
  }
});
*/

    getbootinfo();
}

window.addEventListener("load", boot);




