Class MainWindow
    Private Sub Window_onLoad(sender As Object, e As RoutedEventArgs)
        Dim proc As New Process
        proc.StartInfo.FileName = "getmac"
        proc.StartInfo.Arguments = ""
        proc.StartInfo.CreateNoWindow = True
        proc.StartInfo.UseShellExecute = False
        proc.StartInfo.RedirectStandardOutput = True
        proc.Start()

        Dim output() As String = proc.StandardOutput.ReadToEnd.Split(CChar(vbLf))
        For Each ln As String In output
            If ln.Contains("\Device") Then
                Dim contents As String
                contents = ln.Replace("Tcpip", "NPF")
                ListBox1.Items.Add(Mid(contents, InStr(1, contents, "\Device\NPF_")))
            End If
        Next
        ListBox1.SelectedIndex = 0
        proc.WaitForExit()
    End Sub

    Private Sub LaunchBtn_Click(sender As Object, e As RoutedEventArgs)
        If Not ListBox1.SelectedIndex.Equals(-1) Then
            Dim proc As New Process
            proc.StartInfo.FileName = "ycoe_ppm_user.exe"
            proc.StartInfo.Arguments = ListBox1.SelectedItem.ToString()
            proc.StartInfo.CreateNoWindow = False
            proc.StartInfo.UseShellExecute = True
            proc.StartInfo.RedirectStandardOutput = False
            proc.Start()
            proc.WaitForExit()
        End If
    End Sub
End Class
