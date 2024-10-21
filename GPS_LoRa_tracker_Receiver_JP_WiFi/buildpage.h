/*******************************************************************************
* html and javascript page
 ******************************************************************************/

char page[] PROGMEM = R"(
<!DOCTYPE html><html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GPS機体捜索システム</title>
    <style>
        h1{
            background-color: darkblue;
            color: white;
            font-size: 20pt;
            text-align:center;
        }
        h2{
            font-size: 20pt;
        }
        h3{
            font-size: 12pt;
        }
        .center_layout{
            text-align:center;
        }
        buttun{
          font-size: 20pt;
        }
    </style>
    <script>
        function Reload(){
            var xmlhttp = new XMLHttpRequest();
            var url = '/reload';

            xmlhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    var res = JSON.parse(this.responseText);
   
                    document.getElementById("gLAT").innerHTML=res.gLAT;
                    document.getElementById("gLONG").innerHTML=res.gLONG;
                    document.getElementById("gID").innerHTML=res.gID;
                    document.getElementById("gDATE").innerHTML=res.gDATE;
                    document.getElementById("gTIME").innerHTML=res.gTIME;
                    document.getElementById("gRSSI").innerHTML=res.gRSSI;
                    document.getElementById("gCNT").innerHTML=res.gCNT;
                }
            };
            xmlhttp.open("GET", url, true);
            xmlhttp.send();
        }
        
        function CopyToClipboard(){

            // コピー対象を編集する  
            var ClipBoardText = document.getElementById("gLAT").innerHTML;
            ClipBoardText += ",";
            ClipBoardText += document.getElementById("gLONG").innerHTML;
            
            // 選択しているテキストをクリップボードにコピーする
            // navigator.clipboard.writeText(ClipBoardText); //iOS動かない

            const textarea = document.createElement('textarea');
            textarea.value = ClipBoardText;
            document.body.appendChild(textarea);
            textarea.select();
            const result = document.execCommand('copy');
            document.body.removeChild(textarea);

            alert(ClipBoardText);	
        }

        function JumpToGoogleMap(){
            var url = 'https://www.google.com/maps?q='
            url += document.getElementById("gLAT").innerHTML;
            url += ",";
            url += document.getElementById("gLONG").innerHTML;
            console.log(url);
            window.open(url, '_blank')

        }
    </script>
</head>

<body>
    <h1>GPS機体捜索システム</h1>
    <div class="center_layout">
        <h2><span>経度：</span><span id="gLAT">123.456789</span></h2>
        <h2><span>緯度：</span><span id="gLONG">123.456789</span></h2>
        <h3>ID: <span id="gID">XXXXXXXX</span></h3>
        <h3>DATE: <span id="gDATE">9999/99/99</span></h3>
        <h3>TIME: <span id="gTIME">99:99:99</span></h3>
        <h3>RSSI: <span id="gRSSI">999</span></h3>
        <h3>CNT: <span id="gCNT">999999</span></h3>
    </div>

  <div class="center_layout">
    <button type="button" onclick='Reload()'>RELOAD</button>
    <button type="button" onclick='CopyToClipboard()'>Clipboard</button>
    <button type="button" onclick='JumpToGoogleMap()'>GoogleMap</button>
  </div>
</body>
</html>
)";

