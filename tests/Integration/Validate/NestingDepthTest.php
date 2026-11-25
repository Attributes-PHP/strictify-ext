<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Validate;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\BaseException;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Type;

use function Attributes\Strictify\validate;

/*
 * Payload nesting depth is attacker-controlled and every level consumes C
 * stack, so validate() stops with a catchable exception past the documented
 * limit instead of overflowing the stack.
 */
class DepthNode extends BaseModel
{
    public ?string $label = null;

    public ?DepthNode $child = null;
}

function depth_payload(int $depth): array
{
    $payload = ['label' => 'leaf'];

    for ($i = 0; $i < $depth; $i++) {
        $payload = ['label' => "n{$i}", 'child' => $payload];
    }

    return $payload;
}

describe('validation nesting depth', function () {
    it('validates payloads below the limit', function (int $depth) {
        $result = validate(depth_payload($depth), new DepthNode);

        $seen = 0;
        $node = $result;
        while ($node->child !== null) {
            $node = $node->child;
            $seen++;
        }

        expect($seen)->toBe($depth);
    })->with([1, 100, 511]);

    it('throws a catchable exception past the limit instead of crashing', function (int $depth) {
        expect(fn () => validate(depth_payload($depth), new DepthNode))->toThrow(
            BaseException::class,
            'Maximum validation nesting depth of 512 exceeded',
        );
    })->with([512, 1000, 5000]);

    it('unwinds the depth counter after a deep failure', function () {
        expect(fn () => validate(depth_payload(600), new DepthNode))->toThrow(BaseException::class)
            ->and(validate(depth_payload(3), new DepthNode)->label)
            ->toBe('n2');
    });

    it('guards nested array shapes too', function () {
        $model = new class extends BaseModel {
            #[Sequence(new Sequence(Type::int))]
            public array $value;
        };

        expect(validate(['value' => [[1, 2]]], $model)->value)->toBe([[1, 2]])
            ->and(fn () => validate(['value' => [['x']]], $model))
            ->toThrow(\Attributes\Strictify\Exceptions\ValidationException::class);
    });
});
