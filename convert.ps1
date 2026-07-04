Add-Type -AssemblyName System.Drawing
$pngPath = "$env:USERPROFILE\.gemini\antigravity-ide\brain\5c042177-e05f-45a4-9af2-490ea49c3669\icon_ufo_cdj_1783173231890.png"
$icoPath = "C:\NovaDAW\resources\icon.ico"
[System.IO.Directory]::CreateDirectory("C:\NovaDAW\resources") | Out-Null
$img = [System.Drawing.Image]::FromFile($pngPath)
$fs = New-Object System.IO.FileStream($icoPath, [System.IO.FileMode]::Create)
$icon = [System.Drawing.Icon]::FromHandle((([System.Drawing.Bitmap]$img).GetHicon()))
$icon.Save($fs)
$fs.Close()
$img.Dispose()
