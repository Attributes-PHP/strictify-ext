<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Frameworks;

use Attributes\Strictify\Tests\Benchmark\Scenarios;

use PhpBench\Attributes as Bench;
use Symfony\Component\Validator\Constraints as Assert;
use Symfony\Component\Validator\Constraint;
use Symfony\Component\Validator\Validation;
use Symfony\Component\Validator\Validator\ValidatorInterface;

/**
 * The strictify benchmark scenarios expressed with symfony/validator
 * programmatic constraints. The validator and the constraint trees are
 * built once in the constructor, so subjects measure steady-state
 * validation cost.
 */
#[Bench\OutputTimeUnit('microseconds')]
class SymfonyValidationBench
{
    private ValidatorInterface $validator;

    /** @var array<string, Constraint> */
    private array $constraints = [];

    public function __construct()
    {
        $this->validator = Validation::createValidator();

        $this->constraints['simple'] = new Assert\Collection(fields: [
                'age' => new Assert\Type('integer'),
                'name' => new Assert\Type('string'),
                'email' => new Assert\Type('string'),
                'active' => new Assert\Type('bool'),
                'balance' => new Assert\Type('float'),
            ]);

        // Symfony validates without coercing: the loose scenario accepts
        // the stringly scalars via union types
        $this->constraints['loose'] = new Assert\Collection(fields: [
                'age' => new Assert\Type('numeric'),
                'name' => new Assert\Type('string'),
                'email' => new Assert\Type('string'),
                'active' => new Assert\Type(['bool', 'integer']),
                'balance' => new Assert\Type('numeric'),
            ]);

        $this->constraints['constrained'] = new Assert\Collection(fields: [
                'slug' => new Assert\Regex('/^[a-zA-Z0-9_-]+$/'),
                'label' => new Assert\Regex('/^[\x00-\x7F]*$/'),
                'age' => [new Assert\Type('integer'), new Assert\Range(min: 18, max: 99)],
                'tags' => new Assert\Callback(static function ($value): bool {
                    return is_array($value) && in_array('php', $value, true) && in_array('dev', $value, true);
                }),
                'birthday' => new Assert\DateTime(format: 'Y-m-d'),
                'email' => [new Assert\Type('string'), new Assert\Regex('/\.(com|net)$/')],
                'grade' => new Assert\Choice(choices: ['a', 'b', 'c']),
                'score' => [new Assert\Type('float'), new Assert\LessThanOrEqual(100)],
                'quantity' => [new Assert\Type('integer'), new Assert\GreaterThanOrEqual(1)],
                'packSize' => [new Assert\Type('integer'), new Assert\Callback(static fn ($value): bool => is_int($value) && $value % 5 === 0)],
                'username' => new Assert\Regex('/^[a-z][a-z0-9-]*$/'),
                'name' => [new Assert\Type('string'), new Assert\Length(min: 3, max: 20)],
                'website' => [new Assert\Type('string'), new Assert\Regex('#^https://#')],
                'eventId' => new Assert\Regex('/^[0-7][0-9A-HJKMNP-TV-Z]{25}$/i'),
                'requestId' => new Assert\Uuid(),
            ]);

        $addressFields = [
            'street' => new Assert\Type('string'),
            'city' => new Assert\Type('string'),
        ];
        $this->constraints['nested'] = new Assert\Collection(fields: [
                'id' => new Assert\Type('integer'),
                'addresses' => new Assert\All(new Assert\Collection(fields: $addressFields)),
            ]);

        $wideFields = [];
        for ($i = 1; $i <= 32; $i++) {
            $name = 'field' . str_pad((string) $i, 2, '0', STR_PAD_LEFT);
            $wideFields[$name] = new Assert\Type(match (($i - 1) % 8) {
                0, 1 => 'string',
                2, 3 => 'integer',
                4, 5 => 'float',
                6, 7 => 'bool',
            });
        }
        $this->constraints['wide'] = new Assert\Collection(fields: $wideFields);

        $intList = new Assert\All(new Assert\Type('integer'));
        $this->constraints['matrix'] = new Assert\Collection(fields: [
                'matrix' => new Assert\All($intList),
                'groups' => new Assert\All($intList),
            ]);
        $this->constraints['dict'] = $this->constraints['matrix'];

        $this->constraints['union'] = new Assert\Collection(fields: [
                'mixed' => new Assert\All(new Assert\Type(['integer', 'string', 'boolean'])),
                'coerced' => new Assert\All(new Assert\Type(['integer', 'float'])),
            ]);

        $this->constraints['nine'] = [];
        $nineRules = [
            'profile' => ['username' => 'string', 'email' => 'string', 'loginCount' => 'integer'],
            'signup' => ['user_name' => 'string', 'email' => 'string', 'age' => ['integer', 'numeric']],
            'product' => ['sku' => 'string', 'title' => 'string', 'price' => 'float', 'stock' => 'integer'],
            'session' => ['sessionId' => 'string', 'userId' => 'string', 'rememberMe' => 'bool'],
            'cart' => ['sku' => 'string', 'quantity' => 'integer', 'unitPrice' => 'float'],
            'invoice' => ['number' => 'string', 'customer' => 'string', 'total' => 'float'],
            'review' => ['author' => 'string', 'comment' => 'string', 'rating' => 'integer'],
            'category' => ['slug' => 'string', 'name' => 'string'],
            'tags' => ['name' => 'string', 'tags' => new Assert\All(new Assert\Type('string'))],
        ];
        foreach ($nineRules as $name => $fields) {
            $converted = [];
            foreach ($fields as $field => $type) {
                $converted[$field] = $type instanceof Constraint ? $type : new Assert\Type($type);
            }
            $this->constraints['nine'][$name] = new Assert\Collection(fields: $converted);
        }

        // Warm the metadata caches so subjects measure steady-state cost
        foreach ($this->constraints as $key => $constraint) {
            if (is_array($constraint)) {
                foreach ($constraint as $c) {
                    $this->validator->validate(Scenarios::nineModels()[$key] ?? [], $c);
                }
            } else {
                $this->validator->validate([], $constraint);
            }
        }
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateSimpleModel(): void
    {
        $this->validator->validate(Scenarios::simple(), $this->constraints['simple']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateLooseModel(): void
    {
        $this->validator->validate(Scenarios::loose(), $this->constraints['loose']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModel(): void
    {
        $this->validator->validate(Scenarios::constrained(), $this->constraints['constrained']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModelFailing(): void
    {
        $this->validator->validate(Scenarios::constrained(true), $this->constraints['constrained']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNestedModel(): void
    {
        $this->validator->validate(Scenarios::nested(), $this->constraints['nested']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidate32FieldModel(): void
    {
        $this->validator->validate(Scenarios::wide(), $this->constraints['wide']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateNestedSequences(): void
    {
        $this->validator->validate(Scenarios::matrix(), $this->constraints['matrix']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateDictOfSequences(): void
    {
        $this->validator->validate(Scenarios::dictOfSequences(), $this->constraints['dict']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateUnionExactScalars(): void
    {
        $this->validator->validate(Scenarios::unionScalars(), $this->constraints['union']);
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNineDistinctModels(): void
    {
        foreach (Scenarios::nineModels() as $name => $data) {
            $this->validator->validate($data, $this->constraints['nine'][$name]);
        }
    }
}
