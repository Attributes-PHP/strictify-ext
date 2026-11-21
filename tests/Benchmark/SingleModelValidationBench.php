<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark;

use Attributes\Strictify\Tests\Benchmark\Models\BenchAttributeUser;
use Attributes\Strictify\Tests\Benchmark\Models\BenchEmptyUser;
use Attributes\Strictify\Tests\Benchmark\Models\BenchLooseUser;
use Attributes\Strictify\Tests\Benchmark\Models\BenchOrder;
use Attributes\Strictify\Tests\Benchmark\Models\BenchSimpleUser;
use PhpBench\Attributes as Bench;

use function Attributes\Strictify\validate;

#[Bench\OutputTimeUnit('microseconds')]
class SingleModelValidationBench
{
    private array $simpleData;

    private array $looseData;

    private array $attributeData;

    private array $nestedData;

    private BenchSimpleUser $simpleUser;

    private BenchEmptyUser $emptyUser;

    private BenchLooseUser $looseUser;

    private BenchAttributeUser $attributeUser;

    private BenchOrder $order;

    public function __construct()
    {
        $this->simpleData = Scenarios::simple();
        $this->looseData = Scenarios::loose();
        $this->attributeData = Scenarios::attribute();
        $this->nestedData = Scenarios::nested();

        $this->simpleUser = new BenchSimpleUser;
        $this->emptyUser = new BenchEmptyUser;
        $this->looseUser = new BenchLooseUser;
        $this->attributeUser = new BenchAttributeUser;
        $this->order = new BenchOrder;
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateEmptyModel(): void
    {
        validate([], $this->emptyUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateSimpleModel(): void
    {
        validate($this->simpleData, $this->simpleUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateLooseModel(): void
    {
        validate($this->looseData, $this->looseUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateAttributeModel(): void
    {
        validate($this->attributeData, $this->attributeUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNestedModel(): void
    {
        validate($this->nestedData, $this->order);
    }
}
