# Changelog

## 2.3.0 - 2026-05-15

### 新特性
- **全新浅色默认主题**：基于 Claude.com 暖调编辑风格设计系统（奶油底色 + 珊瑚强调色）
- **暗色模式切换**：设置面板新增"暗色模式"勾选框，可切换回原有暗色 UI
- **TC_* 代理宏架构**：统一主题切换机制，所有绘制代码无需手动判断主题
- **主题感知字体**：浅色模式 Georgia（标题）+ Segoe UI（正文），暗色模式 Microsoft YaHei UI
- **设计令版完整化**：50+ 新增色值常量，覆盖取消扇区、X 按钮、标签等全部 UI 元素

### 重构
- `paint_wheel_preview()` 拆分为 5 个静态绘制函数（header / wheel_body / hub / labels）
- 字体大小常量化：`FONT_SIZE_DISPLAY` / `TITLE` / `BODY` / `RENAME` / `CAPTION`
- 刷子变量语义化重命名：`g_brush_light` → `g_brush_bg_deep` 等

### Bug 修复
- **卡死**：修复轮盘预览中点击固定扇区未拖拽导致鼠标永久捕获（`SetCapture` 时机错误）
- **取消固定错位**：修复 `preview_x_hit_test` 返回 slot 索引而非扇区索引导致 X 按钮与内容不匹配
- **高扇区不可交互**：修复 `slot < pin_count` / `slot < slot_count` 比较逻辑错误，使扇区 5-7 的固定项可正常显示 X 按钮和标签
- **对比度合规**：浅色主题 muted 文字从 3.95:1 提升到 4.8:1（WCAG AA 标准）

## 2.2.0 - 2026-05-03
- 新增键盘方向键轮盘导航（↑↓←→ / PgUp/PgDn/Home/End），无需鼠标即可选择扇区
- 新增右键上下文菜单（复制/固定/编辑/取消固定/删除）
- 新增撤销栈（Undo）支持，删除或取消固定后可一键撤销
- 新增开机自启动选项（注册表 Run 键）
- 新增 Ctrl+F 搜索栏基础框架
- 修复热键吞键问题：Mod=0 时不再拦截修饰键组合（如 Shift+波浪号）
- 修复浮动模式 overlay 超出屏幕导致的扇区黑色空洞
- 修复浮动模式轮盘位置偏移：统一 ClientToScreen 坐标参照系
- 修复控制中心缩放残影：WM_SETREDRAW + RDW_UPDATENOW 原子重绘
- 修复轮盘顶部/底部截断：扩大 overlay 边界并修正边界 clamp
- 将 main.c (2849行) 拆分为 5 个模块：app.h / draw_utils.c / cardlist.c / wheel_preview.c / main.c
- 新增 GitHub Actions CI 工作流（构建 + 自测 + 冒烟测试）
- 新增 AGENTS.md 项目指引文件
- 整理项目目录结构，文档归档至 docs/，截图归档至 assets/screenshots/

## 2.0.0 - 2026-04-18
- 重构主界面与轮盘交互流程
- 新增控制中心与托盘菜单增强
- 新增图标资产生成脚本（Windows/macOS/Linux）
- 补充安装脚本模板与网站介绍页
- 完善仓库文档与发布准备
