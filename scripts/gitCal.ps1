#
# @Author: cubevlmu khfahqp@gmail.com
# @LastEditors: cubevlmu khfahqp@gmail.com
# Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
#

$total = 0
git ls-files '*.cpp' '*.hpp' '*.h' '*.mm' '*.c' '*.cs' | ForEach-Object {
    $lines = (Get-Content $_ | Measure-Object -Line).Lines
    $total += $lines
    Write-Host ("{0,8} {1}" -f $lines, $_)
}
Write-Host ("{0,8} total" -f $total)