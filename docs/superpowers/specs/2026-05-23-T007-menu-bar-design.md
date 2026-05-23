# T007: Menu Bar Design

## Overview
Add QMenuBar to MainFrame with File, Edit, View, Help menus.

## Menu Structure
- **File**: New (Ctrl+N), Open (Ctrl+O), separator, Save (Ctrl+S), Save As (Ctrl+Shift+S), separator, Exit (Ctrl+Q)
- **Edit**: Undo (Ctrl+Z, disabled), Redo (Ctrl+Y, disabled)
- **View**: Fit to Window (Ctrl+0), Actual Size (Ctrl+1), Zoom In (Ctrl++), Zoom Out (Ctrl+-)
- **Help**: About

## Implementation
- New private method `setupMenuBar()` in MainFrame, called from `buildUi()`
- QMenuBar managed by Qt parent-child relationship
- Actions not connected to slots yet (functionality TBD in later tasks)
- Tests verify menu structure, action counts, disabled states, shortcuts
