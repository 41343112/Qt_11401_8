# 修復線上模式對話框阻塞問題 (Dialog Blocking Issue Fix)

## 問題描述 (Problem Description)

**中文:** 線上模式下，當遊戲因投降結束時會彈出對話框。但如果對方沒有關閉對話框就開始新遊戲，對方將無法加入新遊戲。

**English:** In online mode, when a game ends by surrender, a dialog box appears. However, if the opponent doesn't close the dialog box before starting a new game, the opponent cannot join the new game.

## 根本原因 (Root Cause)

使用 `QMessageBox::information()` 創建的對話框是**模態對話框** (modal dialogs)，會阻塞 Qt 的事件循環 (event loop)。當對話框打開時：

1. 應用程式無法處理任何網路訊息
2. 如果對手發送「開始遊戲」訊息，這個訊息會被阻塞
3. 直到對話框關閉後，訊息才會被處理，但此時可能已經太遲

Modal dialogs created with `QMessageBox::information()` block the Qt event loop. When the dialog is open:

1. The application cannot process any network messages
2. If the opponent sends a "start game" message, it gets blocked
3. The message is only processed after the dialog is closed, which may be too late

## 解決方案 (Solution)

將所有遊戲結束相關的對話框改為**非模態對話框** (non-modal dialogs)：

Changed all game-end related dialogs to non-modal dialogs:

### 1. 新增輔助函數 (Added Helper Function)

```cpp
void Qt_Chess::showNonBlockingInfo(const QString& title, const QString& message) {
    // 創建非模態訊息框，避免阻塞事件循環
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(title);
    msgBox->setText(message);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setStandardButtons(QMessageBox::Ok);
    
    // 設定為非模態，不阻塞事件循環
    msgBox->setModal(false);
    
    // 設定對話框關閉時自動刪除
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    
    // 顯示對話框（非阻塞）
    msgBox->show();
}
```

### 2. 更新的對話框 (Updated Dialogs)

以下對話框已從阻塞式改為非阻塞式：

The following dialogs have been changed from blocking to non-blocking:

#### 遊戲結束對話框 (Game End Dialogs)
- 對手投降 (`onSurrenderReceived()`)
- 自己投降 (`onResignClicked()`)
- 收到對手的遊戲結束訊息 (`onGameOverReceived()`)
- 將死 (Checkmate)
- 逼和 (Stalemate)
- 子力不足 (Insufficient material)

#### 超時對話框 (Timeout Dialogs)
- 白方超時
- 黑方超時
- 線上模式和離線模式都已更新

## 技術細節 (Technical Details)

### 模態 vs 非模態對話框 (Modal vs Non-Modal Dialogs)

| 特性 | 模態對話框 | 非模態對話框 |
|------|-----------|-------------|
| 阻塞事件循環 | ✅ 是 | ❌ 否 |
| 可處理網路訊息 | ❌ 否 | ✅ 是 |
| 使用方法 | `exec()` | `show()` |
| 記憶體管理 | 自動 | 需要設定 `Qt::WA_DeleteOnClose` |

| Feature | Modal Dialog | Non-Modal Dialog |
|---------|-------------|------------------|
| Blocks event loop | ✅ Yes | ❌ No |
| Can process network messages | ❌ No | ✅ Yes |
| Method to display | `exec()` | `show()` |
| Memory management | Automatic | Needs `Qt::WA_DeleteOnClose` |

### 關鍵修改 (Key Changes)

**Before (阻塞式):**
```cpp
QMessageBox::information(this, "對手投降", QString("%1投降！%2獲勝！").arg(opponentName).arg(winner));
```

**After (非阻塞式):**
```cpp
showNonBlockingInfo("對手投降", QString("%1投降！%2獲勝！").arg(opponentName).arg(winner));
```

## 影響的檔案 (Affected Files)

1. `src/qt_chess.h` - 新增 `showNonBlockingInfo()` 函數宣告
2. `src/qt_chess.cpp` - 實作 `showNonBlockingInfo()` 並更新所有對話框

## 測試步驟 (Testing Steps)

1. 啟動兩個線上遊戲客戶端
2. 讓其中一方投降
3. 保持其中一方的投降對話框開啟（不關閉）
4. 從另一方開始新遊戲
5. 驗證兩方都能成功加入新遊戲

---

1. Start two online game clients
2. Have one player surrender
3. Keep the surrender dialog open on one client (don't close it)
4. Start a new game from the other client
5. Verify that both clients can successfully join the new game

## 預期行為 (Expected Behavior)

- ✅ 即使對話框開啟，網路訊息仍然可以被處理
- ✅ 玩家可以在對話框開啟時收到並回應「開始遊戲」訊息
- ✅ 不會因為對話框而錯過任何遊戲狀態更新

---

- ✅ Network messages can be processed even when dialogs are open
- ✅ Players can receive and respond to "start game" messages while dialogs are open
- ✅ No game state updates are missed due to dialog blocking

## 代碼審查結果 (Code Review Results)

✅ **通過** - 沒有發現問題

✅ **PASSED** - No issues found

## 安全檢查結果 (Security Check Results)

✅ **通過** - 沒有發現安全問題

✅ **PASSED** - No security issues found

## 相關問題 (Related Issues)

此修復解決了線上模式下因對話框阻塞導致的連線問題。

This fix resolves connection issues in online mode caused by dialog blocking.
