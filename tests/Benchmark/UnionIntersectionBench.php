<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark;

use Attributes\Strictify\Tests\Benchmark\Models\BenchIntersectionPayload;
use Attributes\Strictify\Tests\Benchmark\Models\BenchNestedUnionIntersection;
use Attributes\Strictify\Tests\Benchmark\Models\BenchRegistryService;
use Attributes\Strictify\Tests\Benchmark\Models\BenchUnionModelArm;
use Attributes\Strictify\Tests\Benchmark\Models\BenchUnionPayload;
use Attributes\Strictify\Tests\Benchmark\Models\BenchUnionTree;
use PhpBench\Attributes as Bench;

use function Attributes\Strictify\validate;

/**
 * Union and Intersection attribute scenarios: each subject isolates one
 * arm kind (basic exact match, loose coercion, BaseModel class-arm
 * hydration, interface intersection, composed deep nesting) so a
 * candidate optimization in av_array_typehint_validator.c can be judged
 * against a baseline number.
 *
 * Baselines: `vendor/bin/phpbench run tests/Benchmark/UnionIntersectionBench.php --group=<group>`.
 */
#[Bench\OutputTimeUnit('microseconds')]
class UnionIntersectionBench
{
    private array $mixedData;

    private array $intersectionData;

    private array $nestedData;

    /** @var list<BenchRegistryService> */
    private array $services;

    public function __construct()
    {
        $this->mixedData = Scenarios::unionScalars();

        $this->services = [];
        for ($i = 0; $i < 50; $i++) {
            $this->services[] = new BenchRegistryService('svc-'.$i);
        }

        $this->intersectionData = ['services' => $this->services];

        $this->nestedData = ['payloads' => []];
        for ($i = 0; $i < 40; $i++) {
            $this->nestedData['payloads'][] = ($i % 2) === 0 ? $i : array_slice($this->services, 0, 5);
        }

        // Warm the compiled plans, field-name and spec caches: the
        // scenarios measure steady-state per-call cost, not the
        // first-call compilation
        $this->benchValidateUnionExactScalars();
        $this->benchValidateUnionScalarCoercion();
        $this->benchValidateUnionModelArmHydration();
        $this->benchValidateIntersectionInstances();
        $this->benchValidateDeepUnionTree();
        $this->benchValidateNestedUnionIntersection();
    }

    /**
     * Loose-coercion and hydration scenarios rebuild their raw data per
     * rev: validate() rewrites element buckets in place (and does not
     * respect copy-on-write), so a reused array would exercise the
     * already-coerced or already-hydrated path from the second call on.
     * The rebuild cost is constant across baseline and candidate runs.
     */
    #[Bench\Subject, Bench\Groups(['union'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateUnionExactScalars(): void
    {
        validate($this->mixedData, new BenchUnionPayload);
    }

    #[Bench\Subject, Bench\Groups(['union'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateUnionScalarCoercion(): void
    {
        validate(Scenarios::unionCoerced(), new BenchUnionPayload);
    }

    #[Bench\Subject, Bench\Groups(['union'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateUnionModelArmHydration(): void
    {
        validate(Scenarios::unionModelArm(), new BenchUnionModelArm);
    }

    #[Bench\Subject, Bench\Groups(['intersection'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateIntersectionInstances(): void
    {
        validate($this->intersectionData, new BenchIntersectionPayload);
    }

    #[Bench\Subject, Bench\Groups(['nested-union-intersection'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateDeepUnionTree(): void
    {
        validate(Scenarios::unionTree(), new BenchUnionTree);
    }

    #[Bench\Subject, Bench\Groups(['nested-union-intersection'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateNestedUnionIntersection(): void
    {
        validate($this->nestedData, new BenchNestedUnionIntersection);
    }
}
