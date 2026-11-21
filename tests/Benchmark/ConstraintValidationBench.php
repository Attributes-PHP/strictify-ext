<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark;

use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Tests\Benchmark\Models\BenchConstrainedUser;
use PhpBench\Attributes as Bench;

use function Attributes\Strictify\validate;

#[Bench\OutputTimeUnit('microseconds')]
class ConstraintValidationBench
{
    private array $validData;

    private array $invalidData;

    private BenchConstrainedUser $user;

    public function __construct()
    {
        $this->validData = Scenarios::constrained();
        $this->invalidData = Scenarios::constrained(true);

        $this->user = new BenchConstrainedUser;
    }

    #[Bench\Subject, Bench\Groups(['constraints'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModel(): void
    {
        validate($this->validData, $this->user);
    }

    #[Bench\Subject, Bench\Groups(['constraints'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModelFailing(): void
    {
        try {
            validate($this->invalidData, $this->user);
        } catch (ValidationException) {
            // Expected: exercises the constraint error path
        }
    }
}
