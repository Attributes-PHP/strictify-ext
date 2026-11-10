<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;

it('BaseModel has all required methods', function () {
    $model = new class extends BaseModel {};

    expect(method_exists($model, 'beforeValidation'))->toBeTrue();
    expect(method_exists($model, 'afterValidation'))->toBeTrue();
});
