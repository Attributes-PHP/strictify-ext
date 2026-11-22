<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark;

use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Tests\Benchmark\Models\BenchDateTimeEvent;
use Attributes\Strictify\Tests\Benchmark\Models\BenchErrorForm;
use Attributes\Strictify\Tests\Benchmark\Models\BenchNestedShape;
use Attributes\Strictify\Tests\Benchmark\Models\BenchOrder;
use Attributes\Strictify\Tests\Benchmark\Models\BenchShallowModel;
use Attributes\Strictify\Tests\Benchmark\Models\BenchWideModel;
use PhpBench\Attributes as Bench;

use function Attributes\Strictify\validate;

/**
 * Data-layout scenarios for the review findings: each subject isolates
 * one hot path so a candidate optimization (#5 hot/cold split, #6 spec
 * flattening, per-element path strings, datetime call setup, error
 * template compilation) can be judged against a baseline number.
 *
 * Baselines to record before changing anything: run
 * `vendor/bin/phpbench run tests/Benchmark/DataLayoutBench.php --group=<group>`.
 */
#[Bench\OutputTimeUnit('microseconds')]
class DataLayoutBench
{
    private array $wideData;

    private array $matrixData;

    private array $groupsData;

    private array $ordersData;

    private array $dateTimeData;

    private array $errorData;

    private array $extraKeysData;

    public function __construct()
    {
        $this->wideData = Scenarios::wide();
        $this->matrixData = Scenarios::matrix();
        $this->groupsData = Scenarios::dictOfSequences();
        $this->ordersData = Scenarios::orders();
        $this->dateTimeData = Scenarios::dateTime();
        $this->errorData = Scenarios::errors();
        $this->extraKeysData = Scenarios::extraKeys();

        // Warm the compiled plans, field-name and spec caches: the
        // scenarios measure steady-state per-call cost, not the
        // first-call compilation
        $this->benchValidate32FieldModel();
        $this->benchValidateNestedSequences();
        $this->benchValidateDictOfSequences();
        $this->benchValidateSequenceOfNestedModels();
        $this->benchValidateFourDatetimeCoercions();
        $this->benchValidateEightErrorsPerCall();
        $this->benchValidateThreeFieldModelWithHundredKeys();
    }

    #[Bench\Subject, Bench\Groups(['wide-model'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidate32FieldModel(): void
    {
        validate($this->wideData, new BenchWideModel);
    }

    #[Bench\Subject, Bench\Groups(['nested-shape'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateNestedSequences(): void
    {
        validate($this->matrixData, new BenchNestedShape);
    }

    #[Bench\Subject, Bench\Groups(['nested-shape'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateDictOfSequences(): void
    {
        validate($this->groupsData, new BenchNestedShape);
    }

    #[Bench\Subject, Bench\Groups(['nested-models'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateSequenceOfNestedModels(): void
    {
        validate($this->ordersData, new BenchOrder);
    }

    #[Bench\Subject, Bench\Groups(['datetime-coercion'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateFourDatetimeCoercions(): void
    {
        validate($this->dateTimeData, new BenchDateTimeEvent);
    }

    #[Bench\Subject, Bench\Groups(['error-heavy'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateEightErrorsPerCall(): void
    {
        try {
            validate($this->errorData, new BenchErrorForm);
        } catch (ValidationException) {
            // The expected outcome: eight errors collected, templates
            // and {expected} strings built per field per call
        }
    }

    #[Bench\Subject, Bench\Groups(['extra-keys'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateThreeFieldModelWithHundredKeys(): void
    {
        validate($this->extraKeysData, new BenchShallowModel);
    }
}
