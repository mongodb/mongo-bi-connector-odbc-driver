 <#
 .SYNOPSIS
     Builds an MSI for mongosqld.exe and mongodrdl.exe.
 .DESCRIPTION
     .
 .PARAMETER Platform
     The architecture, "32-bit" or "64-bit".
 #>
 Param(
   [string]$Platform,
   [string]$Server,
   [string]$Port,
   [string]$User,
   [string]$Password,
   [switch]$Atlas,
   [switch]$Local,
   [switch]$LocalSSL,
   [string]$Version,
   [string]$DB = 'test'
 )

Set-strictmode -version latest

# Add-System-OdbcDsn mirrors the Add-OdbcDsn function added in PowerShell 3.0
# for use in PowerShells before 3.0, but only supports System Dsn.
function Add-System-OdbcDsn {
   Param(
      # dsn name
      [string]$Name,
      # name of the ODBC driver
      [string]$Driver,
      # platform should be "32-bit" or "64-bit"
      [string]$Platform,
      # other arguments to the driver, an array of strings
      # of format Property=Value
      [array]$SetPropertyValue
   )
   if ("$Platform" -eq "64-bit") {
       $ODBCRoot = "HKLM:\SOFTWARE\ODBC\"
   } elseif ("$Platform" -eq "32-bit") {
       $ODBCRoot = "HKLM:\SOFTWARE\WOW6432Node\ODBC\"
   } else {
       throw "Unknown Platform '$Platform'"
   }

   $dsnPath = "$ODBCRoot\ODBC.INI\$Name"

   # create DSN registry key
   if(Test-Path $dsnPath) {
       throw "DSN already exists"
   }

   echo "Create DSN path ''$dsnPath'"
   md $dsnPath | Out-Null

   # add properties to DSN
   echo "Add properties to DSN"
   $driverReg = Get-ItemProperty "$ODBCRoot\ODBCINST.INI\$Driver"
   echo "Driver registry = '$driverReg'"
   $driverName = $driverReg.Driver
   echo "Driver name = '$driverName'"

   echo "Set-ItemProperty -Name Driver -Value '$driverName'"
   Set-ItemProperty -Path $dsnPath -Name "Driver" -Value "$driverName" | Out-Null
   foreach($x in $SetPropertyValue) {
       $x_sp = $x.Split("=")
       echo "Set-ItemProperty -Name '$x_sp[0]'-Value '$x_sp[1]'"
       Set-ItemProperty -Path $dsnPath -Name $x_sp[0] -Value $x_sp[1] | Out-Null
   }

   # ODBCDataSources is how an application knows a DSN is available, not really
   # necessary for our integration tests, but nice for completeness.
   $ODBCDataSources = "$ODBCRoot\ODBC.INI\ODBC Data Sources"
   if(!(Test-Path $ODBCDataSources)) {
       md $ODBCDataSources | Out-Null
   }
   Set-ItemProperty -Path $ODBCDataSources -Name $Name -Value $Driver | Out-Null
}

# Remove-System-OdbcDsn mirrors the Add-OdbcDsn function added in PowerShell
# 3.0 for use in PowerShells before 3.0, but only supports System Dsn. Also,
# it does not throw errors on attempting to remove something that does not
# exist, as we use this pessimistically at the beginning of a run in cases
# where the environment is not clean (perhaps due to a previously killed test
# run).
function Remove-System-OdbcDsn {
   Param(
      # name of the dsn to remove
      [string]$Name,
      # platform, should be "32-bit" or "64-bit".
      [string]$Platform
   )
   if ("$Platform" -eq "64-bit") {
       $ODBCRoot = "HKLM:\SOFTWARE\ODBC\"
   } elseif ("$Platform" -eq "32-bit") {
       $ODBCRoot = "HKLM:\SOFTWARE\WOW6432Node\ODBC\"
   } else {
       throw "Unknown Platform '$platform'"
   }
   # remove dsn if it exists.
   $dsnPath = "$ODBCRoot\ODBC.INI\$Name"
   if(Test-Path $dsnPath) {
       Remove-Item -Path $dsnPath -Recurse | Out-Null
   }
   # remove ODBCDataSources entry if it exists.
   $ODBCDataSources = "$ODBCRoot\ODBC.INI\ODBC Data Sources"
   if (Test-Path $ODBCDataSources) {
       $Key = Get-Item -Path $ODBCDataSources
       if ($Key.GetValue($Name, $null) -ne $null) {
           Remove-ItemProperty -Path $ODBCDataSources -Name $Name | Out-Null
       }
   }
}

# Query-ODBC runs an ODBC query against a given DSN.
function Query-ODBC {
    Param(
       # name of the DSN to query
       [string]$Dsn,
       # query to run
       [string]$Query
    )
    $conn = New-Object System.Data.Odbc.OdbcConnection
    $conn.ConnectionString = "DSN=$Dsn"
    $conn.open()
    $cmd = New-object System.Data.Odbc.OdbcCommand($Query,$conn)
    $ds = New-Object system.Data.DataSet
    (New-Object system.Data.odbc.odbcDataAdapter($cmd)).fill($ds) | out-null
    $conn.close()
    $ds.Tables[0]
}

function Test-Query {
    Param (
       # name of the DSN to query
       [string]$Dsn
    )
    if ($Atlas) {
        $query = "select * from greeting where _id = '5c64a48d1c9d44000046008d'"
        $ds = Query-ODBC -Dsn $Dsn -Query $query
        if ($ds._id -ne "5c64a48d1c9d44000046008d") {
               throw "Data not as expected, got _id: $($ds._id), expected '5c64a48d1c9d44000046008d'"
        }
        if ( $ds.message -ne "Hello, world!" ) {
               throw "Data not as expected, got message: $($ds.message), expected 'Hello, world!'"
        }
    } else {
        $query = "select * from information_schema.schemata where schema_name = 'mysql'"
        $ds = Query-ODBC -Dsn $Dsn -Query $query
        if ($ds.CATALOG_NAME -ne 'def') {
            throw 'got incorrect value for CATALOG_NAME'
        }
        if ($ds.SCHEMA_NAME -ne 'mysql') {
            throw 'got incorrect value for SCHEMA_NAME'
        }
        if ($ds.DEFAULT_CHARACTER_SET_NAME -ne 'utf8') {
            throw 'got incorrect value for DEFAULT_CHARACTER_SET_NAME'
        }
        if ($ds.DEFAULT_COLLATION_NAME -ne 'utf8_bin') {
            throw 'got incorrect value for DEFAULT_COLLATION_NAME'
        }
    }
}

# Test-Driver tests with given DSN name, Driver name, and
# array of property values. Cleans up DSNs before
# and after testing, to keep system environment as
# clean as possible.
function Test-Driver {
    Param(
    # name of the dsn to use
    [string]$Dsn,
    # name of the driver to use
    [string]$Driver,
    # platform should be "32-bit" or "64-bit"
    [string]$Platform,
    # SetPropertyValue to pass through to Add-System-OdbcDsn
    [array]$SetPropertyValue
    )

    # remove previous dsn if it exists
    Remove-System-OdbcDsn -Name "$Dsn" -Platform "$Platform"
    # create an ODBC DSN entry. Do not double quote arrays
    # passed through.
    Add-System-OdbcDsn -Name "$Dsn" `
           -Driver "$Driver" `
           -Platform "$Platform" `
           -SetPropertyValue $SetPropertyValue

    # test the driver
    Test-Query -Dsn $Dsn

    # remove dsn to cleanup
    Remove-System-OdbcDsn -Name "$Dsn" -Platform "$Platform"
}

# Install-Mongo-Odbc installs the mongodb-odbc-driver.
function Install-Mongo-Odbc {
   Param(
   # location of the msi
   [string]$Path
   )
   # install the unsigned msi
   # Start-Process with the --wait flag does not seem
   # to work correctly.
   msiexec /i $Path ACCEPT=YES /q /l* install-log.txt
   Start-Sleep -s 5
}

function Test-Local-Connect-Success {
    Param(
      [String]$TestName,
      [String[]]$Props
    )

    echo "...running local connection test '$TestName'"
    $properties = @("Server=$Server",
                    "Port=$Port",
                    "User=$User",
                    "Password=$Password",
                    "Database=$DB")

    foreach($prop in $Props) {
        $properties += $prop
    }

    try {
        foreach($driver in $drivers) {
            Test-Driver -Dsn $dsn -Driver $driver -Platform $Platform -SetPropertyValue $properties
        }
    } catch {
        $err = $Error[0].ToString()
        echo "...test '$TestName' FAILED: connection was rejected"
        echo "error: $err"
        exit 1
    }

    echo "...test '$TestName' SUCCEEDED"
}

function Test-Local-Connect-Failure {
    Param(
      [String]$TestName,
      [String[]]$Props,
      [String]$ExpectedErr
    )

    echo "...running local negative connection test '$TestName'"
    $properties = @("Server=$Server",
                    "Port=$Port",
                    "User=$User",
                    "Password=$Password",
                    "Database=$DB")

    foreach($prop in $Props) {
        $properties += $prop
    }

    try {
        foreach($driver in $drivers) {
            Test-Driver -Dsn $dsn -Driver $driver -Platform $Platform -SetPropertyValue $properties
        }
    } catch {
        $actualErr = $Error[0].ToString()
        if ( $actualErr -like "*$ExpectedErr*" -or $actualErr -like "*SSL connection error*" ) {
            echo "...test '$TestName' SUCCEEDED"
            return
        } else {
            echo "...test '$TestName' SUCCEEDED: But error message did not contain $$ExpectedErr"
            echo "......expected string: $ExpectedErr"
            echo "......actual err message: $actualErr"
            return
        }
    }

    echo "test '$TestName' FAILED: expected connection to be rejected, but it was accepted"
    exit 1
}


function Test-Atlas-Connect-Success {
    Param(
      [String]$TestName,
      [String[]]$Props
    )

    $properties = @("Server=$Server",
                    "Port=$Port",
                    "User=$User",
                    "Password=$Password",
                    "Database=$DB")

    foreach($prop in $Props) {
        $properties += $prop
    }

    echo "...running atlas connection test '$TestName'"
    try {
        foreach($driver in $drivers) {
            Test-Driver -Dsn $dsn -Driver $driver -Platform $Platform -SetPropertyValue $properties
        }
    } catch {
        $err = $Error[0].ToString()
        echo "......test '$TestName' FAILED: connection was rejected"
        echo ".........error: $err"
        exit 1
    }

    echo "......test '$TestName' SUCCEEDED"
}

function Test-Atlas-Connect-Failure {
    Param(
      [String]$TestName,
      [String[]]$Props,
      [String]$ExpectedErr
    )

    $properties = @("Server=$Server",
                    "Port=$Port",
                    "User=$User",
                    "Password=$Password",
                    "Database=$DB")

    foreach($prop in $Props) {
        $properties += $prop
    }

    echo "...running atlas negative connection test '$TestName'"
    try {
        foreach($driver in $drivers) {
            Test-Driver -Dsn $dsn -Driver $driver -Platform $Platform -SetPropertyValue $properties
        }
    } catch {
        $actualErr = $Error[0].ToString()
        if ( $actualErr -like "*$ExpectedErr*" -or $actualErr -like "*SSL connection error*" ) {
            echo "......test '$TestName' SUCCEEDED"
            return
        } else {
            echo "......test '$TestName' SUCCEEDED: But error message did not contain $$ExpectedErr"
            echo ".........expected string: $ExpectedErr"
            echo ".........actual err message: $actualErr"
            return
        }
    }

    echo "...test '$TestName' FAILED: expected connection to be rejected, but it was accepted"
    exit 1
}

# Test all the success and fail cases from success and fail tsv files.
function Run-Test-Cases {
    Param(
      [String]$TestType,
      [String]$SuccessTestsTsv,
      [String]$FailTestsTsv,
      [ScriptBlock]$TestSuccess,
      [ScriptBlock]$TestFail
    )
    # test cases
    if ($SuccessTestsTsv -ne "") {
        echo $TestType" tests that should successfully connect..."
        $tsv = Get-Content $SuccessTestsTsv
        foreach($line in $tsv)
        {
            echo $line
            # clean out empty strings because we might have multiple tabs between items
            $line_sp = $line.Split("`t") | ? { $_ -ne "" }
            $name, $props = $line_sp
            if ($props) {
                # expand any variables in the tsv file using the current environment
                $props = $props | % { $ExecutionContext.InvokeCommand.ExpandString($_) }
            }
            echo "Invoking TestSuccess with'$name' '$props'"
            $TestSuccess.Invoke($name, $props)
        }
    }


    if ($FailTestsTsv -ne "") {
        echo $TestType" tests that should fail to connect..."
        $tsv = Get-Content $FailTestsTsv
        foreach($line in $tsv)
        {
            # clean out empty strings because we might have multiple tabs between items
            $line_sp = $line.Split("`t") | ? { $_ -ne "" }

            $name, $rest = $line_sp
            $expectedErr = $rest[-1]
            $props = $rest[0..($rest.length-2)]

            if ($props) {
                # expand any variables in the tsv file using the current environment
                $props = $props | % { $ExecutionContext.InvokeCommand.ExpandString($_) }
            }
            $expectedErr = $ExecutionContext.InvokeCommand.ExpandString($expectedErr)
            $TestFail.Invoke($name, $props, $expectedErr)
        }
    }
}

$dsn = "TestODBC"
$drivers = @("MongoDB ODBC $Version Unicode Driver",
             "MongoDB ODBC $Version ANSI Driver")

function Get-Script-Directory {
    $scriptInvocation = (Get-Variable MyInvocation -Scope 1).Value
    return Split-Path $scriptInvocation.MyCommand.Path
}

$scriptDir = Get-Script-Directory
$CAPath = "$scriptDir\..\resources"
$digicertCA = "$CAPath\digicert.pem"
$invalidCA = "$CAPath\invalid.pem"
$localCA = "$CAPath\local_ca.pem"

try {
    Install-Mongo-Odbc -Path "mongodb-odbc.msi" -Platform $Platform -Drivers $drivers
} catch {
    $err = $Error[0].ToString()
    echo "Failed to install MongoDB ODBC driver"
    echo "error: $err"
    cat "install-log.txt"
    exit 1
}


# local test cases
if ($Local) {
    Run-Test-Cases -TestType "Local" `
               -SuccessTestsTsv $PSScriptRoot\local-integration-test-success-cases.tsv `
               -FailTestsTsv "" `
               -TestSuccess ${function:Test-Local-Connect-Success} `
               -TestFail ${function:Test-Local-Connect-Failure}
}

# local SSL test cases
if ($LocalSSL) {
    Run-Test-Cases -TestType "LocalSSL" `
               -SuccessTestsTsv $PSScriptRoot\local_ssl-integration-test-success-cases.tsv `
               -FailTestsTsv $PSScriptRoot\local_ssl-integration-test-fail-cases.tsv `
               -TestSuccess ${function:Test-Local-Connect-Success} `
               -TestFail ${function:Test-Local-Connect-Failure}
}

# Atlas test cases
if ($Atlas) {
    Run-Test-Cases -TestType "Atlas" `
               -SuccessTestsTsv $PSScriptRoot\atlas-integration-test-success-cases.tsv `
               -FailTestsTsv $PSScriptRoot\atlas-integration-test-fail-cases.tsv `
               -TestSuccess ${function:Test-Atlas-Connect-Success} `
               -TestFail ${function:Test-Atlas-Connect-Failure}
}
