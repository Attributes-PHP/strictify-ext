<?php

declare(strict_types=1);

define('TESTS_RUNNING', true);

if (! extension_loaded('attributes-strictify')) {
    exit('attributes-strictify extension is not loaded');
}

uses()->group('integration')->in('Integration');
