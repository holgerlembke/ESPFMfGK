var elemento2 = null;
var elemento3 = null;
var elementmsg = null;
var elementfi = null;
var elementti = null;
var elementws = null;

var sektionstrenner = String.fromCharCode(3, 1, 2);
var antworttrenner = String.fromCharCode(2, 1, 3);
var itemtrenner = String.fromCharCode(2, 1, 4);

var foldername = "";

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
  request.onprogress = function (evt) {
    /*        
          var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
          if (lastpercentComplete != percentComplete) {
            lastpercentComplete = percentComplete;
    */
    msgline("Fetching files infos, " + evt.loaded + "  B received.");
    // console.log('Info: '+evt.loaded);
  }
  request.onreadystatechange = AnswerProcessor;
  request.open('GET', '/i?fs=' + param + '&t=' + wantstree + '&pn=' + foldername, true);
  request.send(null);
  waitspinner(true);
}

//000000000000000000000000000
// Handle the getfileinsert answer
function AnswerProcessor() {
  var DONE = this.DONE || 4;
  if (this.readyState === DONE) {
    var t = this.responseText;
    var sections = this.responseText.split(sektionstrenner);

    // Sektion 0: die Ordnerliste 
    var items = sections[0].split(itemtrenner);
    if ((items.length > 0) && (items[0] != "")) {
      var c = 0;
      var s = "<div class=\"fnecc\"><div class=\"fnegc\">";
      while (c < items.length - 1) {
        var entry = items[c].split(":");
        var pathitems = entry[1].split("/");
        s += "<div class=\"fne\" onclick=\"showfolder(" + entry[0] + ",'" + entry[1] + "');\">";

        var level = parseInt(entry[0]);
        for (let i = 0; i < level; i++) {
          s += "&nbsp;&nbsp;";
        }

        s += "<span class=\"fnes fnesl\">" + pathitems[pathitems.length - 1] + "</span></div>";
        c++;
      }
      s += "</div></div>";
      elementti.innerHTML = s;
    } else {
      elementti.innerHTML = "";
    }

    // Sektion 1: die Dateiliste
    var res = sections[1].split(antworttrenner);

    elemento3.innerHTML = res[1];
    elemento2.innerHTML = res[2];

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

      // Makros aufloesen                  
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
    waitspinner(false);
  }
}

//000000000000000000000000000
function getbootinfo() {
  msgline("Get display infos...");
  var request = new XMLHttpRequest();
  request.onprogress = function (evt) {
    msgline("Fetching display infos, " + evt.loaded + "  B received.");
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

    // ESPxWebFlMgr2::Backgroundcolor
    if (res[0] != "") {
      var c = document.getElementsByClassName('background');
      for (i = 0; i < c.length; i++) {
        c[i].style.backgroundColor = res[0];
      }
    }
    // ESPxWebFlMgr2::ExtraHTMLfoot
    if (res[1] != "") {
      var d = document.getElementById("foot");
      d.innerHTML = res[1];
    }

    // Seitentitle
    if (res[2] != "") {
      if (document.title != res[2]) {
        document.title = res[2];
      }
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
  editxhr.onreadystatechange = function () {
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

  xhr.onreadystatechange = function () {
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
  xhr.upload.onprogress = function (evt) {
    if (evt.lengthComputable) {
      var percentComplete = Math.round((evt.loaded / evt.total) * 100.0);
      if (lastpercentComplete != percentComplete) {
        lastpercentComplete = percentComplete;
        msgline("Please wait. Upload progress " + percentComplete + "%");
        // console.log('Progress: '+percentComplete);
      }
    }
  }
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
function waitspinner(ison) {
  if (ison) {
    elementws.style.visibility = "visible";
  } else {
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
function boot() {
  // Does lookup need time?  
  elemento2 = document.getElementById("o2");
  elemento3 = document.getElementById('o3');
  elementmsg = document.getElementById('msg');
  elementfi = document.getElementById("fi");
  elementti = document.getElementById("ti");
  elementws = document.getElementById("wait");

  getbootinfo();
}

//->
window.onload = boot;
