[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$Installer)

$ErrorActionPreference = "Stop"
$installerPath = (Resolve-Path -LiteralPath $Installer).Path
$installPath = Join-Path $env:RUNNER_TEMP "parsinama-installed"
$installLog = Join-Path $env:RUNNER_TEMP "parsinama-install.log"
$env:PARSINAMA_CATALOG_PATH = $null
$installerProcess = Start-Process -FilePath $installerPath -ArgumentList @(
    "/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART",
    "/DIR=`"$installPath`"", "/LOG=`"$installLog`""
) -Wait -PassThru
if ($installerProcess.ExitCode -ne 0) {
    if (Test-Path -LiteralPath $installLog) { Get-Content -LiteralPath $installLog -Tail 80 }
    throw "Installer failed with exit code $($installerProcess.ExitCode)"
}
$app = Join-Path $installPath "bin\leomoon-parsinama.exe"
$catalog = Join-Path $installPath "bin\data\parsinama-catalog.sqlite"
$driver = Join-Path $installPath "bin\sqldrivers\qsqlite.dll"
foreach ($path in @($app, $catalog, $driver)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing installed file: $path" }
}
Push-Location $env:RUNNER_TEMP
try {
    $env:QT_LOGGING_TO_CONSOLE = "1"
    $stdoutLog = Join-Path $env:RUNNER_TEMP "parsinama-installed-stdout.log"
    $stderrLog = Join-Path $env:RUNNER_TEMP "parsinama-installed-stderr.log"
    $appProcess = Start-Process -FilePath $app -ArgumentList "--smoke-test" -RedirectStandardOutput $stdoutLog -RedirectStandardError $stderrLog -Wait -PassThru
    if ($appProcess.ExitCode -ne 0) {
        if (Test-Path -LiteralPath $stdoutLog) { Get-Content -LiteralPath $stdoutLog -Tail 80 }
        if (Test-Path -LiteralPath $stderrLog) { Get-Content -LiteralPath $stderrLog -Tail 80 }
        throw "Installed app smoke test failed with exit code $($appProcess.ExitCode)"
    }
} finally {
    Pop-Location
    $uninstaller = Join-Path $installPath "unins000.exe"
    if (Test-Path -LiteralPath $uninstaller) {
        $uninstallProcess = Start-Process -FilePath $uninstaller -ArgumentList @(
            "/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART"
        ) -Wait -PassThru
        if ($uninstallProcess.ExitCode -ne 0) {
            throw "Uninstaller failed with exit code $($uninstallProcess.ExitCode)"
        }
    }
}
