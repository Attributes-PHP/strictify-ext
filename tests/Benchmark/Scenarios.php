<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark;

/**
 * Input data shared by all benchmark suites (strictify and the framework
 * comparison benches), so every implementation validates identical input.
 */
final class Scenarios
{
    public static function simple(): array
    {
        return [
            'age' => 30,
            'name' => 'Andre',
            'email' => 'andre@example.com',
            'active' => true,
            'balance' => 1250.50,
        ];
    }

    /**
     * Stringly-typed scalars: frameworks that coerce accept them as-is,
     * Symfony is given relaxed union types instead (it never coerces).
     */
    public static function loose(): array
    {
        return [
            'age' => '30',
            'name' => 'Andre',
            'email' => 'andre@example.com',
            'active' => 1,
            'balance' => '1250.50',
        ];
    }

    public static function constrained(bool $fail = false): array
    {
        $data = [
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

        if ($fail) {
            $data['age'] = 17; // Between(18, 99) fails
        }

        return $data;
    }

    public static function nested(): array
    {
        $addresses = [];
        $cities = ['Porto', 'Lisbon', 'Braga', 'Faro', 'Aveiro', 'Coimbra', 'Setubal', 'Guimaraes', 'Viseu', 'Evora'];
        foreach ($cities as $i => $city) {
            $addresses[] = ['street' => 'Street ' . $i, 'city' => $city];
        }

        return ['id' => 42, 'addresses' => $addresses];
    }

    public static function wide(): array
    {
        $data = [];
        for ($i = 1; $i <= 32; $i++) {
            $name = 'field' . str_pad((string) $i, 2, '0', STR_PAD_LEFT);
            $data[$name] = match (($i - 1) % 8) {
                0, 1 => 'value-' . $i,
                2, 3 => $i,
                4, 5 => $i + 0.5,
                6, 7 => $i % 2 === 1,
            };
        }

        return $data;
    }

    public static function matrix(): array
    {
        $data = ['matrix' => [], 'groups' => ['seed' => [1]]];
        for ($i = 0; $i < 20; $i++) {
            $data['matrix'][] = range($i * 50, $i * 50 + 49);
        }

        return $data;
    }

    public static function dictOfSequences(): array
    {
        $data = ['matrix' => [[1]], 'groups' => []];
        for ($i = 0; $i < 20; $i++) {
            $data['groups']['group-' . $i] = range(0, 49);
        }

        return $data;
    }

    public static function unionScalars(): array
    {
        $mixed = [];
        for ($i = 0; $i < 100; $i++) {
            $mixed[] = match ($i % 3) {
                0 => $i,
                1 => 'item-' . $i,
                2 => ($i % 2) === 0,
            };
        }

        return ['mixed' => $mixed, 'coerced' => []];
    }

    public static function attribute(): array
    {
        return [
            'user_name' => 'Andre',
            'age' => 30,
            'tags' => ['php', 'extension', 'validation'],
        ];
    }

    public static function orders(int $count = 20): array
    {
        $data = ['id' => 1, 'addresses' => []];
        for ($i = 0; $i < $count; $i++) {
            $data['addresses'][] = ['street' => 'Street ' . $i, 'city' => 'City ' . $i];
        }

        return $data;
    }

    public static function dateTime(): array
    {
        $stamp = '2026-10-09T12:30:00+00:00';

        return [
            'createdAt' => $stamp,
            'updatedAt' => $stamp,
            'deletedAt' => $stamp,
            'archivedAt' => $stamp,
        ];
    }

    public static function errors(): array
    {
        return [
            'username' => 42,
            'age' => 'not an int',
            'score' => true,
            'active' => 3.14,
            'note' => false,
            'when' => 'not a datetime',
            'email' => 99,
            'tags' => 'oops',
        ];
    }

    public static function extraKeys(): array
    {
        $data = [
            'name' => 'andre',
            'count' => 7,
            'ratio' => 1.5,
        ];
        for ($i = 0; $i < 97; $i++) {
            $data['unknown' . $i] = $i;
        }

        return $data;
    }

    public static function unionCoerced(): array
    {
        $data = ['mixed' => [], 'coerced' => []];
        for ($i = 0; $i < 100; $i++) {
            $data['coerced'][] = ($i % 2) === 0 ? '30' : '250';
        }

        return $data;
    }

    public static function unionModelArm(): array
    {
        $data = ['items' => []];
        for ($i = 0; $i < 30; $i++) {
            $data['items'][] = ($i % 3) === 2
                ? $i
                : ['street' => 'Street ' . $i, 'city' => 'City ' . $i];
        }

        return $data;
    }

    public static function unionTree(): array
    {
        // Self-referencing union tree: 4 levels, each node mixing scalar
        // arms with hydrated child nodes
        $node = null;
        $node = static function (int $depth) use (&$node): array {
            $children = [1, 'leaf'];
            if ($depth > 0) {
                for ($i = 0; $i < 4; $i++) {
                    $children[] = $node($depth - 1);
                }
            }

            return ['name' => 'node-' . $depth, 'children' => $children];
        };
        $data = ['name' => 'root', 'children' => []];
        for ($i = 0; $i < 6; $i++) {
            $data['children'][] = ($i % 2) === 0 ? $i : $node(3);
        }

        return $data;
    }

    /**
     * @return array<string, array<string, mixed>>
     */
    public static function nineModels(): array
    {
        return [
            'profile' => ['username' => 'andre', 'email' => 'andre@example.com', 'loginCount' => 42],
            'signup' => ['user_name' => 'andre', 'email' => 'andre@example.com', 'age' => '30'],
            'product' => ['sku' => 'SKU-1', 'title' => 'Validation extension', 'price' => 99.9, 'stock' => 10],
            'session' => ['sessionId' => 'abc123', 'userId' => 'user-1', 'rememberMe' => true],
            'cart' => ['sku' => 'SKU-1', 'quantity' => 2, 'unitPrice' => 49.95],
            'invoice' => ['number' => 'INV-2026-001', 'customer' => 'Acme Inc', 'total' => 1250.50],
            'review' => ['author' => 'andre', 'comment' => 'Fast validation', 'rating' => 5],
            'category' => ['slug' => 'php-extensions', 'name' => 'PHP Extensions'],
            'tags' => ['name' => 'validation', 'tags' => ['php', 'c', 'extension']],
        ];
    }
}
