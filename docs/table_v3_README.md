# Table V3 - 高性能命令管理システム

Table V3は、WebAssembly移行システム用の高性能な命令管理システムです。以下の3つの主要なデータ構造を提供します：

## 主要データ構造

### 1. アドレスマップ (AddressMap)
u32キーとu64バリューを対応付けるためのハッシュテーブルベースの構造です。

**特徴:**
- u32キー → u64バリューの高速検索
- O(1)平均時間での挿入・検索・削除
- 動的サイズ調整
- エントリの更新機能

**用途:**
- 命令アドレスと物理アドレスのマッピング
- オフセット値の変換テーブル
- 任意のu32 -> u64マッピング

### 2. チェックポイント禁止リスト (CheckpointForbiddenList)
特定の命令アドレスでのチェックポイント作成を禁止するためのハッシュ集合です。

**特徴:**
- 高速な存在チェック (O(1)平均)
- メモリ効率的な実装
- 動的な追加・削除

**用途:**
- 内部命令でのチェックポイント禁止
- クリティカルセクションの保護
- マイグレーション制御

### 3. 状態管理キュー (StateManagementQueue)
未確定の命令位置を一時的に保持し、後続命令生成時に対応付けを確定するためのFIFOキューです。

**特徴:**
- FIFO順序による処理
- 確定/未確定状態の管理
- 柔軟な状態更新

**用途:**
- 遅延バインディング
- 分岐命令の処理
- 動的コード生成

## API概要

### アドレスマップの使用

```c
#include "wasmig/table_v3.h"

// アドレスマップの使用
AddressMap* map = address_map_create(128);

// u32キーとu64バリューをセット
uint32_t key = 100;
uint64_t value = 0x1000000000ULL;
address_map_set(map, key, value);

// バリューを取得
uint64_t retrieved_value;
if (address_map_get(map, key, &retrieved_value)) {
    printf("キー %u -> バリュー %lu\n", key, retrieved_value);
}

// エントリを削除
address_map_remove(map, key);

address_map_destroy(map);
```

### チェックポイント禁止リストの使用

```c
// 禁止リストの使用
CheckpointForbiddenList* list = forbidden_list_create(128);
InstructionAddress addr = {1, 100};

// アドレスを禁止リストに追加
forbidden_list_add(list, addr);

// 禁止されているかチェック
if (forbidden_list_contains(list, addr)) {
    printf("チェックポイント禁止\n");
}

forbidden_list_destroy(list);
```

### 状態管理キューの使用

```c
// 状態キューの使用
StateManagementQueue* queue = state_queue_create();
InstructionAddress addr = {1, 100};
IntermediateRepresentation ir = {0x20, 5, 0};

// 未確定命令を追加
state_queue_enqueue(queue, addr, ir);

// 命令を確認
state_queue_confirm_pending(queue, addr);

// 命令を取得
InstructionAddress out_addr;
IntermediateRepresentation out_ir;
state_queue_dequeue(queue, &out_addr, &out_ir);

state_queue_destroy(queue);
}

// エントリを削除
address_map_remove(map, key);

address_map_destroy(map);
```

## ビルド方法

```bash
# プロジェクトのビルド
mkdir build && cd build
cmake ..
make -j4

# テストの実行
./tests/wasmig_tests --gtest_filter="*TableV3*"

# 使用例の実行
./examples/table_v3_example
```

## パフォーマンス特性

| 操作 | 時間計算量 | 空間計算量 |
|------|------------|------------|
| キー→バリュー検索 | O(1)平均 | O(n) |
| キー→バリュー設定 | O(1)平均 | O(n) |
| キー削除 | O(1)平均 | O(n) |
| 禁止リスト確認 | O(1)平均 | O(m) |
| キュー操作 | O(1) | O(k) |

- n: 登録されたマッピング数
- m: 禁止リストのエントリ数  
- k: キューのエントリ数

## ファイル構成

```
include/wasmig/table_v3.h       # ヘッダーファイル
src/table_v3.cpp                # 実装ファイル
tests/test_table_v3.cpp         # テストファイル
examples/table_v3_example.cpp   # 使用例
```

## デバッグ機能

各データ構造には、デバッグ用の印刷機能が提供されています：

```c
address_map_print(map);          // アドレスマップの内容表示
forbidden_list_print(list);     // 禁止リストの内容表示
state_queue_print(queue);       // キューの内容表示
```

## ログ出力

spdlogを使用した詳細なデバッグログが利用可能です。ログレベルを`debug`に設定することで、各操作の詳細な情報を確認できます。

## 注意事項

- メモリ管理: 作成したデータ構造は必ず対応する`destroy`関数で解放してください
- スレッドセーフティ: 現在の実装はスレッドセーフではありません。マルチスレッド環境では適切な同期が必要です
- エラーハンドリング: 関数の戻り値を適切にチェックしてください

## 今後の拡張予定

- [ ] スレッドセーフな実装
- [ ] 動的リサイズ機能の改善
- [ ] メモリプールによる最適化
- [ ] 永続化機能
- [ ] メトリクス収集機能
