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
        $this->validData = [
            'slug' => 'user-1_name',
            'label' => 'hello',
            'age' => 30,
            'tags' => ['php', 'dev', 'oss'],
            'birthday' => '1994-01-01',
            'email' => 'andre@example.com',
            'grade' => 'b',
            'score' => 99.5,
            'quantity' => 1,
            'packSize' => 20,
            'username' => 'user-name',
            'name' => 'andre',
            'website' => 'https://example.com',
            'eventId' => '01ARZ3NDEKTSV4RRFFQ69G5FAV',
            'requestId' => '123e4567-e89b-12d3-a456-426614174000',
        ];

        $this->invalidData = $this->validData;
        $this->invalidData['age'] = 17; // Between(18, 99) fails

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
