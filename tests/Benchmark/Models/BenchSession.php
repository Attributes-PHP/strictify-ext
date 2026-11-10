<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchSession extends BaseModel
{
    public string $sessionId;

    public string $userId;

    public bool $rememberMe;
}
