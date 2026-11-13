<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Constraints\AlphaDash;
use Attributes\Strictify\Fields\Constraints\Ascii;
use Attributes\Strictify\Fields\Constraints\Between;
use Attributes\Strictify\Fields\Constraints\Contains;
use Attributes\Strictify\Fields\Constraints\DateTime;
use Attributes\Strictify\Fields\Constraints\EndsWith;
use Attributes\Strictify\Fields\Constraints\In;
use Attributes\Strictify\Fields\Constraints\Max;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\Constraints\MultipleOf;
use Attributes\Strictify\Fields\Constraints\Regex;
use Attributes\Strictify\Fields\Constraints\Size;
use Attributes\Strictify\Fields\Constraints\StartsWith;
use Attributes\Strictify\Fields\Constraints\Ulid;
use Attributes\Strictify\Fields\Constraints\Uuid;

class BenchConstrainedUser extends BaseModel
{
    #[AlphaDash]
    public string $slug;

    #[Ascii]
    public string $label;

    #[Between(18, 99)]
    public int $age;

    #[Contains('php', 'dev')]
    public array $tags;

    #[DateTime(format: 'Y-m-d')]
    public string $birthday;

    #[EndsWith('.com', '.net')]
    public string $email;

    #[In('a', 'b', 'c')]
    public string $grade;

    #[Max(100)]
    public float $score;

    #[Min(1)]
    public int $quantity;

    #[MultipleOf(5)]
    public int $packSize;

    #[Regex('/^[a-z][a-z0-9-]*$/')]
    public string $username;

    #[Size(min: 3, max: 20)]
    public string $name;

    #[StartsWith('https://')]
    public string $website;

    #[Ulid]
    public string $eventId;

    #[Uuid]
    public string $requestId;
}
