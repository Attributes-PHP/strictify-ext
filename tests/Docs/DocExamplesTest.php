<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Docs;

use FilesystemIterator;
use RecursiveDirectoryIterator;
use RecursiveIteratorIterator;
use RuntimeException;

/*
 * Runs the runnable PHP code blocks of the documentation, so the examples
 * can never drift from the actual behavior. A fenced block is executed when
 * it carries a marker comment:
 *
 *   // doctest: run           the block must run cleanly (exit code 0)
 *   // doctest: expect-throw  the block must fail with a ValidationException
 *
 * Each block runs in its own PHP process (with the extension loaded), so
 * blocks may freely reuse class names. Blocks without a marker are treated
 * as illustrative fragments and skipped.
 */
it('runs every doctest-marked php code block in the docs', function () {
    $root = dirname(__DIR__, 2);
    $docsDir = $root . '/docs';
    $extension = $root . '/modules/attributes_strictify.so';
    $extensionFlag = is_file($extension) ? ' -d extension=' . escapeshellarg($extension) : '';

    $blocks = [];
    $files = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($docsDir, FilesystemIterator::SKIP_DOTS));
    foreach ($files as $file) {
        if ($file->getExtension() !== 'md') {
            continue;
        }
        $markdown = file_get_contents($file->getPathname());
        if (preg_match_all('/```php\n(.*?)```/s', $markdown, $matches)) {
            foreach ($matches[1] as $index => $code) {
                if (str_contains($code, '// doctest: run') || str_contains($code, '// doctest: expect-throw')) {
                    $blocks[$file->getPathname() . ' (block #' . $index . ')'] = $code;
                }
            }
        }
    }

    expect($blocks)->not->toBeEmpty('no doctest-marked php blocks found under docs/');

    foreach ($blocks as $name => $code) {
        $expectThrow = str_contains($code, '// doctest: expect-throw');

        $tmp = tempnam(sys_get_temp_dir(), 'strictify-doctest-') . '.php';
        file_put_contents($tmp, $code);
        $command = escapeshellarg(PHP_BINARY) . $extensionFlag . ' ' . escapeshellarg($tmp) . ' 2>&1';
        $output = [];
        $exitCode = 0;
        exec($command, $output, $exitCode);
        unlink($tmp);
        $outputText = implode("\n", $output);

        if ($expectThrow) {
            if ($exitCode === 0 || !str_contains($outputText, 'ValidationException')) {
                throw new RuntimeException("$name should throw a ValidationException, got exit code $exitCode:\n$outputText");
            }
        } elseif ($exitCode !== 0) {
            throw new RuntimeException("$name failed with exit code $exitCode:\n$outputText");
        }
    }
});
