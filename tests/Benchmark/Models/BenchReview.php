<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchReview extends BaseModel
{
    public string $author;

    public string $comment;

    public int $rating;
}
