Option Explicit

MsgBox "���s���܂�"

Dim objWShell, objFso, objAutoOk
Dim objIE
Dim ret, iPlay
Dim bDayEnd
Do While True
    ' 1���̎n�܂�.
    Set objWShell = WScript.CreateObject("WScript.Shell")
    Set objFso = createObject("Scripting.FileSystemObject")
    Set objAutoOk = objWShell.Exec("cscript " & objFso.getParentFolderName(WScript.ScriptFullName) & "\autook.vbs")

    bDayEnd = False

    Do While True

        Set objIE = CreateObject("InternetExplorer.Application")
        objIE.Visible = True
        objIE.Navigate "https://paiza.jp/paizajack/mypage"
        Call WaitFor(3)
        Call IEWait(objIE)
        Call WaitFor(3)
   
        iPlay = 0
        Do While True
            If objIE.locationUrl = "https://paiza.jp/paizajack" Then
                ' ���O�C������.
                ret = IELinkClickByHrefIn(objIE, "https://paiza.jp/user_sessions/new_cbox" )
                If ret Then
                    Call WaitFor(5)
                    ret = IEButtonClick(objIE, "���O�C������" )
                    If ret Then
                        Call WaitFor(5)
                        Call IEWait(objIE)
                        Call WaitFor(5)
                    End If
                End If
                iPlay = iPlay + 1
            ElseIf InStr( objIE.locationUrl, "/summary_in_battle" ) > 0 Or InStr( objIE.locationUrl, "/summary" ) > 0 Then
                Call WaitFor(10)
                ' ���g���C�����N����.
                ret = IELinkClickByHrefIn(objIE, "/retry" )
                If Not ret Then
                    ' �Ȃ�������Ƃ肠�����X�V.
                    objIE.Refresh
                End If
                iPlay = iPlay + 1
                Call IEWait(objIE)
            ElseIf InStr( objIE.locationUrl, "paizajack/play" ) > 0 And _ 
                   InStr( objIE.locationUrl, "/retry" ) = 0 And _ 
                   InStr( objIE.locationUrl, "paizajack/play/" ) = 0 Then
                ' �R�[�h���s�����N����.
                ret = IELinkClickByOnClick(objIE, "hand_in_code();" )
                If Not ret Then
                    ' �Ȃ�������Ƃ肠�����X�V.
                    objIE.Refresh
                    iPlay = iPlay + 1
                Else
                    Call WaitFor(15)' OK �{�^���҂�.
                    Call IEWait(objIE)
                    Call WaitFor(15)' ���ʑ҂�.
                End If
            Else
                If IEisExistDiv( objIE, "���������̒�o������𒴂������ߒ�o�o���܂���" ) Then
                    iPlay = 999999
                    bDayEnd = True
                End If
                objIE.navigate "https://paiza.jp/paizajack/play"
            End If

            Call WaitFor(1)

            If iPlay > 150 Then
                ' �u���E�U�ċN������.
                ' �����Ɠ����u���E�U�v���Z�X�g���ƕs����ɂȂ�̂�.
                Call WaitFor(45)
                Exit Do
            End If
        Loop

        objIE.Quit
        Call WaitFor(5)
        Set objIE = Nothing

        If bDayEnd Then
            Exit Do
        End If
    Loop
   
    ' 1���̏I���.
    Call objAutoOk.Terminate()
    Call WaitFor(5)
    Set objAutoOk = Nothing
    Set objWShell = Nothing
    Set objFso = Nothing

    '���̓���2���܂Ŗ���.
    Call WaitTime( 2, 0 )
Loop


WScript.Quit (0)

'�{�^���������֐�
Public Function IEButtonClick(ByRef objIE, buttonValue)
    Dim objInput
   
    On Error Resume Next
    For Each objInput In objIE.Document.getElementsByTagName("INPUT")
        If objInput.Value = buttonValue Then
            objInput.Click
            On Error GoTo 0
            Exit For
        End If
    Next
    On Error GoTo 0
End Function
 
'�{�^���������֐�
Public Function IELinkClickByOnClick(ByRef objIE, linkValue)
    IELinkClickByOnClick = False
    Dim objA
   
    On Error Resume Next
    For Each objA In objIE.Document.getElementsByTagName("A")
        If objA.getAttribute( "OnClick" ) = linkValue Then
            IELinkClickByOnClick = True
            objA.Click
            On Error GoTo 0
            Exit For
        End If
    Next
    On Error GoTo 0
End Function
 
'�{�^���������֐�
Public Function IELinkClickByHrefIn(ByRef objIE, linkValue)
    IELinkClickByHrefIn = False
    Dim objA
   
    On Error Resume Next
    For Each objA In objIE.Document.getElementsByTagName("A")
        If InStr( objA.href, linkValue ) > 0 Then
            IELinkClickByHrefIn = True
            objA.Click
            On Error GoTo 0
            Exit For
        End If
    Next
    On Error GoTo 0
End Function
 
'������e��DIV������
Public Function IEisExistDiv(ByRef objIE, divValue)
    IEisExistDiv = False
    Dim objDiv
   
    On Error Resume Next
    For Each objDiv In objIE.Document.getElementsByTagName("DIV")
        If InStr( objDiv.innerText, divValue ) > 0 Then
            IEisExistDiv = True
            On Error GoTo 0
            Exit For
        End If
    Next
    On Error GoTo 0
End Function
 
'IE��ҋ@����֐�
Function IEWait(ByRef objIE)
    Do While objIE.Document.readyState <> "complete"
        WScript.Sleep 100
    Loop
End Function
 
'�w�肵���b������~����֐�
Function WaitFor(ByVal second)
    Dim futureTime
    
    futureTime = DateAdd("s", second, Now)
    Do While Now < futureTime
        WScript.Sleep 100
    Loop
End Function

'�w�肵�����ԂɂȂ�̂�҂֐�
Function WaitTime(ByVal hour, ByVal minute)
    Dim timePrev, timeCheck, timeNow
    timePrev = Time
    timeCheck = TimeSerial( hour, minute, 0 )
   
    Do While True
        timeNow = Time
        If timePrev < timeCheck And timeNow >= timeCheck Then
            Exit Do
        End If
        timePrev = timeNow
        WScript.Sleep 1000 * 60 * 15 ' 15������
    Loop
End Function
