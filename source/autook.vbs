Option Explicit


Dim objWShell, bExist
Set objWShell = WScript.CreateObject("WScript.Shell")
'ShellWindowから1つずつ取得して処理
Do While True
    bExist = objWShell.AppActivate("Web ページからのメッセージ")
    If bExist Then
        objWShell.SendKeys "{ENTER}"
    End If
    WScript.Sleep 3000
Loop

WScript.Quit (0)
   
