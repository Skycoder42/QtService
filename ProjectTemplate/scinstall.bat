@echo off
sc create %{ProjectName} binPath= "C:\\path\\to\\%{TargetName}.exe --backend windows" start= demand displayname= "%{ProjectName} Service" || exit /B 1
sc description %{ProjectName} "The %{ProjectName} Service" || exit /B 1
