Option Explicit


Dim objWShell, bExist
Set objWShell = WScript.CreateObject("WScript.Shell")
'ShellWindow����1���擾���ď���
Do While True
    bExist = objWShell.AppActivate("Web �y�[�W����̃��b�Z�[�W")
    If bExist Then
        objWShell.SendKeys "{ENTER}"
    End If
    WScript.Sleep 3000
Loop

WScript.Quit (0)
   
