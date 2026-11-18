# ============================================================================
# RailEase End-to-End Automated Test Suite
# Tests all backend REST endpoints and business rules against running server
# ============================================================================

param(
    [string]$BaseUrl = "http://127.0.0.1:8080",
    [switch]$SkipRestart = $false
)

$PassCount = 0
$FailCount = 0

function Assert-Test {
    param(
        [string]$TestName,
        [bool]$Condition,
        [string]$Details = ""
    )
    if ($Condition) {
        Write-Host "  [PASS] $TestName" -ForegroundColor Green
        $script:PassCount++
    } else {
        Write-Host "  [FAIL] $TestName - $Details" -ForegroundColor Red
        $script:FailCount++
    }
}

Write-Host "`n===========================================================" -ForegroundColor Cyan
Write-Host "             RAILEASE COMPREHENSIVE TEST SUITE              " -ForegroundColor Cyan
Write-Host "===========================================================`n" -ForegroundColor Cyan

# Pre-flight: Auto-start server if not already running
$serverRunning = $false
try {
    $h = Invoke-RestMethod -Uri "$BaseUrl/api/health" -Method Get -TimeoutSec 2
    if ($h.status -eq "healthy") { $serverRunning = $true }
} catch {}

if (-not $serverRunning) {
    Write-Host "Server not running at $BaseUrl. Launching server.exe 8080..." -ForegroundColor Cyan
    $workDir = if ($PSScriptRoot) { $PSScriptRoot } else { (Get-Location).Path }
    Start-Process -FilePath "$workDir\server.exe" -ArgumentList "8080" -WorkingDirectory $workDir -WindowStyle Hidden
    for ($i = 0; $i -lt 15; $i++) {
        Start-Sleep -Milliseconds 800
        try {
            $h = Invoke-RestMethod -Uri "$BaseUrl/api/health" -Method Get -TimeoutSec 2
            if ($h.status -eq "healthy") { $serverRunning = $true; break }
        } catch {}
    }
}

# ----------------------------------------------------------------------------
# 1. Health check
# ----------------------------------------------------------------------------
Write-Host "TEST 1: Server Health Check" -ForegroundColor Yellow
$health = Invoke-RestMethod -Uri "$BaseUrl/api/health" -Method Get
Assert-Test "Health endpoint returns 200 & healthy status" ($health.status -eq "healthy")

# ----------------------------------------------------------------------------
# 2. Listing initial trains
# ----------------------------------------------------------------------------
Write-Host "`nTEST 2: List Trains & Baseline Count" -ForegroundColor Yellow
$initialTrainsRes = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Get
Assert-Test "Returns success=true" ($initialTrainsRes.success -eq $true)
Assert-Test "Returns at least 3 initial trains" ($initialTrainsRes.count -ge 3)
$initialCount = $initialTrainsRes.count

# Dynamically find next available train number (default 12004)
$testTrainNumber = 12004
while (($initialTrainsRes.trains | Where-Object { $_.trainNumber -eq $testTrainNumber }) -ne $null) {
    $testTrainNumber++
}

# ----------------------------------------------------------------------------
# 3. Add New Valid Train (POST /api/trains)
# ----------------------------------------------------------------------------
Write-Host "`nTEST 3: Add New Valid Express Train (Train #$testTrainNumber)" -ForegroundColor Yellow
$newTrainBody = @{
    trainNumber = $testTrainNumber
    trainName = "Vande Bharat Express"
    source = "Delhi"
    destination = "Varanasi"
    totalSeats = 10
    fare = 1250
} | ConvertTo-Json

try {
    $addTrainRes = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $newTrainBody -ContentType "application/json"
    Assert-Test "Add train returns success=true" ($addTrainRes.success -eq $true)
    Assert-Test "Returned train number is $testTrainNumber" ($addTrainRes.train.trainNumber -eq $testTrainNumber)
    Assert-Test "Total seats initialized to 10" ($addTrainRes.train.totalSeats -eq 10)
    Assert-Test "Available seats initialized to 10" ($addTrainRes.train.availableSeats -eq 10)
    Assert-Test "Booked seats is 0" ($addTrainRes.train.bookedSeats -eq 0)
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Add train failed with status $statusCode" $false
}

# Verify in full list
$afterAddTrainsRes = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Get
Assert-Test "Train list count incremented by 1" ($afterAddTrainsRes.count -eq ($initialCount + 1))
$foundTrainInList = ($afterAddTrainsRes.trains | Where-Object { $_.trainNumber -eq $testTrainNumber })
Assert-Test "Train $testTrainNumber found in train list" ($foundTrainInList -ne $null)

# ----------------------------------------------------------------------------
# 4. Retrieve newly added train by number (GET /api/trains/:number)
# ----------------------------------------------------------------------------
Write-Host "`nTEST 4: Retrieve Newly Added Train by Train Number" -ForegroundColor Yellow
$getTrainRes = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber" -Method Get
Assert-Test "GET /api/trains/$testTrainNumber returns success=true" ($getTrainRes.success -eq $true)
Assert-Test "Train name matches 'Vande Bharat Express'" ($getTrainRes.train.trainName -eq "Vande Bharat Express")
Assert-Test "Source is Delhi and Destination is Varanasi" ($getTrainRes.train.source -eq "Delhi" -and $getTrainRes.train.destination -eq "Varanasi")
Assert-Test "Fare is Rs. 1250" ($getTrainRes.train.fare -eq 1250)

# ----------------------------------------------------------------------------
# 5. Search for newly added train by route (GET /api/trains/search)
# ----------------------------------------------------------------------------
Write-Host "`nTEST 5: Search for Newly Added Train by Route" -ForegroundColor Yellow
$searchVaranasi = Invoke-RestMethod -Uri "$BaseUrl/api/trains/search?source=Delhi&destination=Varanasi" -Method Get
Assert-Test "Search Delhi -> Varanasi returns at least 1 match" ($searchVaranasi.count -ge 1)
Assert-Test "Matched train list contains $testTrainNumber" (($searchVaranasi.trains | Where-Object { $_.trainNumber -eq $testTrainNumber }) -ne $null)

# ----------------------------------------------------------------------------
# 6. Reject duplicate train number (POST /api/trains) -> HTTP 409
# ----------------------------------------------------------------------------
Write-Host "`nTEST 6: Reject Duplicate Train Number (409 Conflict)" -ForegroundColor Yellow
try {
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $newTrainBody -ContentType "application/json"
    Assert-Test "Duplicate train addition should have failed" $false
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Duplicate train returns HTTP 409 Conflict" ($statusCode -eq 409)
}

# ----------------------------------------------------------------------------
# 7. Reject invalid train creation data -> HTTP 400
# ----------------------------------------------------------------------------
Write-Host "`nTEST 7: Reject Invalid Train Data (400 Bad Request)" -ForegroundColor Yellow

# 7a: Negative train number
try {
    $bad = @{ trainNumber = -10; trainName = "Bad Train"; source = "A"; destination = "B"; totalSeats = 10; fare = 100 } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $bad -ContentType "application/json"
    Assert-Test "Negative train number should fail" $false
} catch {
    Assert-Test "Negative train number returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

# 7b: Empty name
try {
    $bad = @{ trainNumber = 9901; trainName = "   "; source = "A"; destination = "B"; totalSeats = 10; fare = 100 } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $bad -ContentType "application/json"
    Assert-Test "Blank train name should fail" $false
} catch {
    Assert-Test "Blank train name returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

# 7c: Empty station
try {
    $bad = @{ trainNumber = 9902; trainName = "Fast Train"; source = ""; destination = "B"; totalSeats = 10; fare = 100 } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $bad -ContentType "application/json"
    Assert-Test "Empty source station should fail" $false
} catch {
    Assert-Test "Empty source station returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

# 7d: Same source and destination
try {
    $bad = @{ trainNumber = 9903; trainName = "Loop Express"; source = "Mumbai"; destination = "Mumbai"; totalSeats = 10; fare = 100 } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $bad -ContentType "application/json"
    Assert-Test "Same source and destination should fail" $false
} catch {
    Assert-Test "Same source and destination returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

# 7e: Invalid seat count (0 or negative)
try {
    $bad = @{ trainNumber = 9904; trainName = "Zero Train"; source = "Delhi"; destination = "Agra"; totalSeats = 0; fare = 100 } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $bad -ContentType "application/json"
    Assert-Test "Zero seat capacity should fail" $false
} catch {
    Assert-Test "Zero seat capacity returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

# 7f: Invalid fare (negative)
try {
    $bad = @{ trainNumber = 9905; trainName = "Free Train"; source = "Delhi"; destination = "Agra"; totalSeats = 10; fare = -50 } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Post -Body $bad -ContentType "application/json"
    Assert-Test "Negative fare should fail" $false
} catch {
    Assert-Test "Negative fare returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

# ----------------------------------------------------------------------------
# 8. Capacity: Increase seat capacity on train with NO bookings ($testTrainNumber: 10 -> 15)
# ----------------------------------------------------------------------------
Write-Host "`nTEST 8: Increase Seat Capacity on Train with No Bookings" -ForegroundColor Yellow
$capPayload15 = @{ totalSeats = 15 } | ConvertTo-Json
$capRes15 = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body $capPayload15 -ContentType "application/json"
Assert-Test "Increase capacity returns success=true" ($capRes15.success -eq $true)
Assert-Test "Total seats updated to 15" ($capRes15.train.totalSeats -eq 15)
Assert-Test "Available seats matches new capacity 15" ($capRes15.train.availableSeats -eq 15)
Assert-Test "Booked seats is 0" ($capRes15.train.bookedSeats -eq 0)

# ----------------------------------------------------------------------------
# 9. Bookings on new train & Capacity increase with ACTIVE bookings
# ----------------------------------------------------------------------------
Write-Host "`nTEST 9: Active Bookings & Capacity Increase" -ForegroundColor Yellow

# Book passenger 1
$bookP1 = @{ trainNumber = $testTrainNumber; passengerName = "Rahul Sharma" } | ConvertTo-Json
$p1Res = Invoke-RestMethod -Uri "$BaseUrl/api/bookings" -Method Post -Body $bookP1 -ContentType "application/json"
$pnrP1 = $p1Res.pnr

# Book passenger 2
$bookP2 = @{ trainNumber = $testTrainNumber; passengerName = "Priya Singh" } | ConvertTo-Json
$p2Res = Invoke-RestMethod -Uri "$BaseUrl/api/bookings" -Method Post -Body $bookP2 -ContentType "application/json"
$pnrP2 = $p2Res.pnr

$trainAfterBook = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber" -Method Get
Assert-Test "2 bookings created successfully (PNR $pnrP1, $pnrP2)" ($pnrP1 -ne $null -and $pnrP2 -ne $null)
Assert-Test "Available seats dropped from 15 to 13" ($trainAfterBook.train.availableSeats -eq 13)
Assert-Test "Booked seats is now 2" ($trainAfterBook.train.bookedSeats -eq 2)

# Now increase capacity from 15 to 20 with active bookings
$capPayload20 = @{ totalSeats = 20 } | ConvertTo-Json
$capRes20 = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body $capPayload20 -ContentType "application/json"
Assert-Test "Capacity increased to 20 with active bookings" ($capRes20.train.totalSeats -eq 20)
Assert-Test "Booked seats remains exactly 2" ($capRes20.train.bookedSeats -eq 2)
Assert-Test "Available seats is correctly recalculated to 18 (20 - 2)" ($capRes20.train.availableSeats -eq 18)

# ----------------------------------------------------------------------------
# 10. Decrease capacity down to active bookings count (20 -> 2)
# ----------------------------------------------------------------------------
Write-Host "`nTEST 10: Decrease Capacity Down to Active Bookings Count" -ForegroundColor Yellow
$capPayload2 = @{ totalSeats = 2 } | ConvertTo-Json
$capRes2 = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body $capPayload2 -ContentType "application/json"
Assert-Test "Decrease capacity to active bookings (2) succeeds" ($capRes2.success -eq $true)
Assert-Test "Total seats is now 2" ($capRes2.train.totalSeats -eq 2)
Assert-Test "Booked seats is 2" ($capRes2.train.bookedSeats -eq 2)
Assert-Test "Available seats is 0 (fully booked)" ($capRes2.train.availableSeats -eq 0)

# ----------------------------------------------------------------------------
# 11. Reject capacity reduction below active bookings (2 -> 1) -> HTTP 409
# ----------------------------------------------------------------------------
Write-Host "`nTEST 11: Reject Capacity Reduction Below Active Bookings (409 Conflict)" -ForegroundColor Yellow
try {
    $capPayload1 = @{ totalSeats = 1 } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body $capPayload1 -ContentType "application/json"
    Assert-Test "Reducing capacity below active bookings should fail" $false
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Reducing capacity below active bookings returns HTTP 409 Conflict" ($statusCode -eq 409)
}

# Verify capacity was NOT changed
$verifyUnchanged = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber" -Method Get
Assert-Test "Train capacity remains 2" ($verifyUnchanged.train.totalSeats -eq 2)

# ----------------------------------------------------------------------------
# 12. Cancelled reservations do not prevent capacity reduction
# ----------------------------------------------------------------------------
Write-Host "`nTEST 12: Cancelled Reservations Do Not Prevent Capacity Reduction" -ForegroundColor Yellow
# Cancel Priya Singh's ticket (PNR P2)
$cancelP2Res = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/$pnrP2/cancel" -Method Post
Assert-Test "Cancelled Priya's ticket successfully" ($cancelP2Res.success -eq $true)

$trainAfterP2Cancel = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber" -Method Get
Assert-Test "Active booked seats is now 1" ($trainAfterP2Cancel.train.bookedSeats -eq 1)
Assert-Test "Available seats increased to 1" ($trainAfterP2Cancel.train.availableSeats -eq 1)

# Now capacity reduction to 1 should SUCCEED because active bookings = 1 (cancelled reservation excluded)
$capRes1 = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body $capPayload1 -ContentType "application/json"
Assert-Test "Capacity reduced to 1 successfully (cancelled ticket ignored)" ($capRes1.success -eq $true)
Assert-Test "Total seats is now 1" ($capRes1.train.totalSeats -eq 1)
Assert-Test "Booked seats is 1" ($capRes1.train.bookedSeats -eq 1)
Assert-Test "Available seats is 0" ($capRes1.train.availableSeats -eq 0)

# ----------------------------------------------------------------------------
# 13. Verify existing bookings remain intact after capacity changes
# ----------------------------------------------------------------------------
Write-Host "`nTEST 13: Passenger Bookings Remain Intact After Capacity Changes" -ForegroundColor Yellow
$p1Booking = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/$pnrP1" -Method Get
Assert-Test "Rahul's booking is still CONFIRMED" ($p1Booking.booking.status -eq "CONFIRMED")
Assert-Test "Passenger name is still Rahul Sharma" ($p1Booking.booking.passengerName -eq "Rahul Sharma")
Assert-Test "Train is still $testTrainNumber" ($p1Booking.booking.trainNumber -eq $testTrainNumber)

$p2Booking = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/$pnrP2" -Method Get
Assert-Test "Priya's booking remains CANCELLED" ($p2Booking.booking.status -eq "CANCELLED")

# ----------------------------------------------------------------------------
# 14. Non-existent train capacity change (404) & Invalid capacity (400)
# ----------------------------------------------------------------------------
Write-Host "`nTEST 14: Non-Existent Train Capacity (404) & Invalid Value (400)" -ForegroundColor Yellow
try {
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains/99999/capacity" -Method Patch -Body (@{ totalSeats = 10 } | ConvertTo-Json) -ContentType "application/json"
    Assert-Test "Non-existent train capacity change should fail" $false
} catch {
    Assert-Test "Non-existent train returns HTTP 404" ($_.Exception.Response.StatusCode.value__ -eq 404)
}

try {
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body (@{ totalSeats = 0 } | ConvertTo-Json) -ContentType "application/json"
    Assert-Test "Zero capacity should fail" $false
} catch {
    Assert-Test "Zero capacity returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

try {
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body (@{ totalSeats = -5 } | ConvertTo-Json) -ContentType "application/json"
    Assert-Test "Negative capacity should fail" $false
} catch {
    Assert-Test "Negative capacity returns HTTP 400" ($_.Exception.Response.StatusCode.value__ -eq 400)
}

# ----------------------------------------------------------------------------
# 15. Capacity expansion and booking on new train
# ----------------------------------------------------------------------------
Write-Host "`nTEST 15: Capacity Expansion & Booking on New Train" -ForegroundColor Yellow
# Expand capacity to 5
$capPayload5 = @{ totalSeats = 5 } | ConvertTo-Json
$capRes5 = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber/capacity" -Method Patch -Body $capPayload5 -ContentType "application/json"
Assert-Test "Capacity set to 5 (available: 4)" ($capRes5.train.availableSeats -eq 4)

# Book ticket for Anita Desai
$bookP3 = @{ trainNumber = $testTrainNumber; passengerName = "Anita Desai" } | ConvertTo-Json
$p3Res = Invoke-RestMethod -Uri "$BaseUrl/api/bookings" -Method Post -Body $bookP3 -ContentType "application/json"
$pnrP3 = $p3Res.pnr
Assert-Test "Anita booked on $testTrainNumber (PNR $pnrP3)" ($pnrP3 -ne $null)

$trainAfterP3 = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber" -Method Get
Assert-Test "Available seats decremented to 3" ($trainAfterP3.train.availableSeats -eq 3)
Assert-Test "Booked seats incremented to 2" ($trainAfterP3.train.bookedSeats -eq 2)

# Cancel Anita's ticket & verify seat restored
$cancelP3Res = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/$pnrP3/cancel" -Method Post
Assert-Test "Cancelled Anita's ticket successfully" ($cancelP3Res.success -eq $true)

$trainAfterP3Cancel = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber" -Method Get
Assert-Test "Available seats restored to 4" ($trainAfterP3Cancel.train.availableSeats -eq 4)
Assert-Test "Booked seats back to 1" ($trainAfterP3Cancel.train.bookedSeats -eq 1)

# ----------------------------------------------------------------------------
# 16. Persistence Across Server Restart
# ----------------------------------------------------------------------------
Write-Host "`nTEST 16: Persistence Across Server Restart" -ForegroundColor Yellow
if (-not $SkipRestart) {
    Write-Host "  Stopping server process..." -ForegroundColor Gray
    Get-Process server -ErrorAction SilentlyContinue | Stop-Process -Force
    Start-Sleep -Seconds 2

    Write-Host "  Restarting server (server.exe 8080)..." -ForegroundColor Gray
    $workDir = if ($PSScriptRoot) { $PSScriptRoot } else { (Get-Location).Path }
    Start-Process -FilePath "$workDir\server.exe" -ArgumentList "8080" -WorkingDirectory $workDir -WindowStyle Hidden
    
    $ready = $false
    for ($i = 0; $i -lt 15; $i++) {
        Start-Sleep -Milliseconds 800
        try {
            $h = Invoke-RestMethod -Uri "$BaseUrl/api/health" -Method Get -TimeoutSec 2
            if ($h.status -eq "healthy") {
                $ready = $true
                break
            }
        } catch {}
    }
    Assert-Test "Server restarted and responding to health check" $ready

    if ($ready) {
        $restartedTrain = Invoke-RestMethod -Uri "$BaseUrl/api/trains/$testTrainNumber" -Method Get
        Assert-Test "Train $testTrainNumber persisted after restart" ($restartedTrain.success -eq $true)
        Assert-Test "Total capacity persisted as 5" ($restartedTrain.train.totalSeats -eq 5)
        Assert-Test "Available seats persisted as 4" ($restartedTrain.train.availableSeats -eq 4)
        Assert-Test "Booked seats persisted as 1" ($restartedTrain.train.bookedSeats -eq 1)

        # Check booking persistence
        $restartedBooking = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/$pnrP1" -Method Get
        Assert-Test "Booking PNR $pnrP1 persisted as CONFIRMED" ($restartedBooking.booking.status -eq "CONFIRMED")
    }
} else {
    Write-Host "  [SKIP] Server restart test skipped via parameter." -ForegroundColor Gray
}

# ----------------------------------------------------------------------------
# 17. Verify original endpoints & baseline functionality still pass
# ----------------------------------------------------------------------------
Write-Host "`nTEST 17: Verify Original Endpoints & Search" -ForegroundColor Yellow
$pnr1001 = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/1001" -Method Get
Assert-Test "PNR 1001 lookup succeeds" ($pnr1001.success -eq $true)
Assert-Test "PNR 1001 status is CANCELLED" ($pnr1001.booking.status -eq "CANCELLED")
Assert-Test "PNR 1001 passenger is Aditya" ($pnr1001.booking.passengerName -eq "Aditya")

$pnr1003 = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/1003" -Method Get
Assert-Test "PNR 1003 lookup succeeds" ($pnr1003.success -eq $true)
Assert-Test "PNR 1003 status is CONFIRMED" ($pnr1003.booking.status -eq "CONFIRMED")

$searchDelhiAgra = Invoke-RestMethod -Uri "$BaseUrl/api/trains/search?source=Delhi&destination=Agra" -Method Get
Assert-Test "Delhi -> Agra search matches Shatabdi" ($searchDelhiAgra.trains[0].trainName -eq "Shatabdi Express")

try {
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/99999" -Method Get
    Assert-Test "Invalid PNR lookup fails" $false
} catch {
    Assert-Test "Invalid PNR returns HTTP 404" ($_.Exception.Response.StatusCode.value__ -eq 404)
}

# ----------------------------------------------------------------------------
# 18. Live Dashboard Statistics Check
# ----------------------------------------------------------------------------
Write-Host "`nTEST 18: Live Dashboard Statistics Integrity" -ForegroundColor Yellow
$finalStats = Invoke-RestMethod -Uri "$BaseUrl/api/stats" -Method Get
Assert-Test "Stats returns success=true" ($finalStats.success -eq $true)
Assert-Test "Total trains count >= 4 (includes added 12004)" ($finalStats.totalTrains -ge 4)
Assert-Test "Confirmed bookings count is accurate (> 0)" ($finalStats.confirmedBookings -gt 0)
Assert-Test "Cancelled bookings count is accurate (> 0)" ($finalStats.cancelledBookings -gt 0)
Assert-Test "Total seats >= available seats" ($finalStats.totalSeats -ge $finalStats.availableSeats)

# ----------------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------------
Write-Host "`n===========================================================" -ForegroundColor Cyan
$summaryColor = "Green"
if ($FailCount -gt 0) { $summaryColor = "Red" }
Write-Host "TEST SUITE SUMMARY: $PassCount PASSED, $FailCount FAILED" -ForegroundColor $summaryColor
Write-Host "===========================================================`n" -ForegroundColor Cyan

if ($FailCount -gt 0) {
    exit 1
} else {
    exit 0
}
