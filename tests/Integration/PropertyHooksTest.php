<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

// Property hooks are PHP 8.4+ syntax: the file containing the hooked
// models must not be parsed on older versions, so it is only required
// (and therefore only parsed) when running on PHP 8.4+.
if (PHP_VERSION_ID >= 80_400) {
    require_once __DIR__.'/PropertyHooks/hooks.php';
}
