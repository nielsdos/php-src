<?php
if (PHP_INT_SIZE !== 8) die('skip Only for 64-bit');
if (getenv('SKIP_SLOW_TESTS')) die('skip slow test');
// Copied from file_get_contents_file_put_contents_5gb.phpt
function get_system_memory(): int|float|false
{
    if (strtoupper(substr(PHP_OS, 0, 3)) === 'WIN') {
        // Windows-based memory check
        @exec('wmic OS get FreePhysicalMemory', $output);
        if (isset($output[1])) {
            return ((int)trim($output[1])) * 1024;
        }
    } else {
        // Unix/Linux-based memory check
        $memInfo = @file_get_contents("/proc/meminfo");
        if ($memInfo) {
            preg_match('/MemFree:\s+(\d+) kB/', $memInfo, $matches);
            return $matches[1] * 1024; // Convert to bytes
        }
    }
    return false;
}
if (get_system_memory() < 4 * 1024 * 1024 * 1024) {
    die('skip Reason: Insufficient RAM (less than 4GB)');
}
