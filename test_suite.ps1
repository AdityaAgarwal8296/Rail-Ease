# ============================================================================
# RailEase End-to-End Automated Test Suite
# Tests all backend REST endpoints and business rules against running server
# ============================================================================

$BaseUrl = "http://localhost:8080"
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

# 1. Health check
Write-Host "TEST 1: Server Health Check" -ForegroundColor Yellow
$health = Invoke-RestMethod -Uri "$BaseUrl/api/health" -Method Get
Assert-Test "Health endpoint returns 200 & healthy status" ($health.status -eq "healthy")

# 2. Listing trains
Write-Host "`nTEST 2: List Trains" -ForegroundColor Yellow
$trainsRes = Invoke-RestMethod -Uri "$BaseUrl/api/trains" -Method Get
Assert-Test "Returns success=true" ($trainsRes.success -eq $true)
Assert-Test "Returns at least 3 trains" ($trainsRes.count -ge 3)
Assert-Test "Train 12001 exists" (($trainsRes.trains | Where-Object { $_.trainNumber -eq 12001 }) -ne $null)

# 3. Searching routes
Write-Host "`nTEST 3: Route Search" -ForegroundColor Yellow
$searchDelhiAgra = Invoke-RestMethod -Uri "$BaseUrl/api/trains/search?source=Delhi&destination=Agra" -Method Get
Assert-Test "Delhi -> Agra search returns 1 train" ($searchDelhiAgra.count -eq 1)
Assert-Test "Matched train is Shatabdi Express" ($searchDelhiAgra.trains[0].trainName -eq "Shatabdi Express")

$searchUnknown = Invoke-RestMethod -Uri "$BaseUrl/api/trains/search?source=Nowhere&destination=Somewhere" -Method Get
Assert-Test "Non-existent route search returns 0 trains" ($searchUnknown.count -eq 0)

# 4. View existing PNR
Write-Host "`nTEST 4: View Existing PNR (1001)" -ForegroundColor Yellow
$pnrRes = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/1001" -Method Get
Assert-Test "PNR 1001 lookup succeeds" ($pnrRes.success -eq $true)
Assert-Test "Passenger is Aditya" ($pnrRes.booking.passengerName -eq "Aditya")
Assert-Test "Status is CONFIRMED" ($pnrRes.booking.status -eq "CONFIRMED")

# 5. Invalid PNR lookup
Write-Host "`nTEST 5: Invalid PNR Lookup (99999)" -ForegroundColor Yellow
try {
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/99999" -Method Get
    Assert-Test "Invalid PNR lookup should return 404" $false
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Invalid PNR returns HTTP 404" ($statusCode -eq 404)
}

# 6. Book a ticket & check seat decrease
Write-Host "`nTEST 6: Book Ticket & Decrement Available Seats" -ForegroundColor Yellow
$train12001Before = Invoke-RestMethod -Uri "$BaseUrl/api/trains/12001" -Method Get
$availSeatsBefore = $train12001Before.train.availableSeats

$bookBody = @{
    trainNumber = 12001
    passengerName = "Vikram Malhotra"
} | ConvertTo-Json

$bookRes = Invoke-RestMethod -Uri "$BaseUrl/api/bookings" -Method Post -Body $bookBody -ContentType "application/json"
Assert-Test "Booking created with success=true" ($bookRes.success -eq $true)
$newPNR = $bookRes.pnr
Assert-Test "PNR is generated (> 1001)" ($newPNR -ge 1002)

$train12001After = Invoke-RestMethod -Uri "$BaseUrl/api/trains/12001" -Method Get
$availSeatsAfter = $train12001After.train.availableSeats
Assert-Test "Available seats decreased by 1 ($availSeatsBefore -> $availSeatsAfter)" ($availSeatsAfter -eq ($availSeatsBefore - 1))

# 7. Cancel ticket & verify seat restored
Write-Host "`nTEST 7: Cancel Ticket & Restore Available Seats" -ForegroundColor Yellow
$cancelRes = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/$newPNR/cancel" -Method Post
Assert-Test "Cancellation returns success=true" ($cancelRes.success -eq $true)
Assert-Test "Booking status is now CANCELLED" ($cancelRes.booking.status -eq "CANCELLED")

$train12001Restored = Invoke-RestMethod -Uri "$BaseUrl/api/trains/12001" -Method Get
Assert-Test "Available seats restored to original ($availSeatsBefore)" ($train12001Restored.train.availableSeats -eq $availSeatsBefore)

# 8. Cancel already cancelled ticket
Write-Host "`nTEST 8: Re-cancel Already Cancelled Ticket" -ForegroundColor Yellow
try {
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/bookings/$newPNR/cancel" -Method Post
    Assert-Test "Re-cancelling should fail" $false
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Re-cancelling returns HTTP 400" ($statusCode -eq 400)
}

# 9. Invalid train number booking
Write-Host "`nTEST 9: Booking with Non-Existent Train Number" -ForegroundColor Yellow
try {
    $invalidTrainBody = @{ trainNumber = 77777; passengerName = "Ghost Rider" } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/bookings" -Method Post -Body $invalidTrainBody -ContentType "application/json"
    Assert-Test "Invalid train booking should fail" $false
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Invalid train number returns 404/400" ($statusCode -eq 404 -or $statusCode -eq 400)
}

# 10. Missing/Blank passenger name
Write-Host "`nTEST 10: Blank Passenger Name Validation" -ForegroundColor Yellow
try {
    $blankNameBody = @{ trainNumber = 12001; passengerName = "   " } | ConvertTo-Json
    $null = Invoke-RestMethod -Uri "$BaseUrl/api/bookings" -Method Post -Body $blankNameBody -ContentType "application/json"
    Assert-Test "Blank passenger name should fail" $false
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Blank passenger name returns HTTP 400" ($statusCode -eq 400)
}

# 11. Malformed JSON request
Write-Host "`nTEST 11: Malformed JSON Handling" -ForegroundColor Yellow
try {
    $null = Invoke-WebRequest -Uri "$BaseUrl/api/bookings" -Method Post -Body "NOT_VALID_JSON" -ContentType "application/json" -UseBasicParsing
    Assert-Test "Malformed JSON should fail" $false
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Assert-Test "Malformed JSON returns HTTP 400" ($statusCode -eq 400)
}

# 12. Dashboard stats verification
Write-Host "`nTEST 12: Live Dashboard Statistics" -ForegroundColor Yellow
$stats = Invoke-RestMethod -Uri "$BaseUrl/api/stats" -Method Get
Assert-Test "Stats returns success=true" ($stats.success -eq $true)
Assert-Test "Total trains matches train count" ($stats.totalTrains -eq $trainsRes.count)
Assert-Test "Confirmed bookings is positive" ($stats.confirmedBookings -ge 1)
Assert-Test "Cancelled bookings is at least 1" ($stats.cancelledBookings -ge 1)

# Summary
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
