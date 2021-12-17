@echo off

rem param 1 pause flag          (true/false)
rem param 2 platform            (win)
rem param 3 database file       (windows)
rem param 4 debug out           (true/false)
rem param 5 check time          (true/false)
rem param 6 errors allowed      (true/false)
rem param 7 output path postfix ()
rem param 8 only convert changed files (true/false)

title Windows: Convert PC
config\convert_with_param.bat true win windows false true false win false
