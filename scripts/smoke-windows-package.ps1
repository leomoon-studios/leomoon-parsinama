[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$Installer)

$ErrorActionPreference = "Stop"
$installerPath = (Resolve-Path -LiteralPath $Installer).Path
$installPath = Join-Path $env:RUNNER_TEMP "parsinama-installed"
$installLog = Join-Path $env:RUNNER_TEMP "parsinama-install.log"
$env:PARSINAMA_CATALOG_PATH = $null
& $installerPath "/VERYSILENT" "/SUPPRESSMSGBOXES" "/NORESTART" "/DIR=$installPath" "/LOG=$installLog"
if ($LASTEXITCODE -ne 0) { throw "Installer failed with exit code $LASTEXITCODE" }
$app = Join-Path $installPath "bin\leomoon-parsinama.exe"
$catalog = Join-Path $installPath "bin\data\parsinama-catalog.sqlite"
$driver = Join-Path $installPath "bin\sqldrivers\qsqlite.dll"
foreach ($path in @($app, $catalog, $driver)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing installed file: $path" }
}
Push-Location $env:RUNNER_TEMP
try {
    & $app --smoke-test
    if ($LASTEXITCODE -ne 0) { throw "Installed app smoke test failed with exit code $LASTEXITCODE" }
} finally {
    Pop-Location
    $uninstaller = Join-Path $installPath "unins000.exe"
    if (Test-Path -LiteralPath $uninstaller) {
        & $uninstaller "/VERYSILENT" "/SUPPRESSMSGBOXES" "/NORESTART"
    }
}
