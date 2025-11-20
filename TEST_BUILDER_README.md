# Test Builder Documentation

## Overview

The Test Builder is a new page in the Flecs Test Runner that allows users to visually create unit tests without manually writing JSON files. It provides an intuitive interface for:

1. **Configuring test parameters**
2. **Setting up systems to run**
3. **Defining initial entity states**
4. **Specifying expected results**
5. **Generating and executing tests**

## Features

### Test Configuration
- Test name input
- Multiple systems configuration with execution counts

### Entity Builder
- **Initial State**: Define entities and their components before test execution
- **Expected State**: Define how entities should look after system execution
- Dynamic component field management
- Module detection for automatic `using` statements

### JSON Generation
- Real-time preview of generated JSON
- Download functionality for generated test files
- Format matches existing test structure

### Test Execution
- Direct integration with existing test runner
- Automatic test creation and execution
- Navigation to results page after test creation

## Usage Flow

1. **Open Test Builder**: Click "Test Builder" in the navigation
2. **Configure Test**: Enter test name and add systems
3. **Set Initial State**: Add entities and components with initial values
4. **Define Expected State**: Specify how entities should appear after execution
5. **Preview/Save**: Review generated JSON or save to file
6. **Execute**: Run the test directly or use the saved JSON file

## JSON Format

The test builder generates JSON files compatible with the existing test runner:

```json
{
  "name": "testMovement",
  "systems": [
    {
      "name": "modules::movement::move",
      "timesToRun": 1
    }
  ],
  "scriptActual": "using modules.movement\n\nTestEntity {\n    Position: {x: 10.0, y: 20.0}\n    Velocity: {x: 1.0, y: 2.0}\n}",
  "scriptExpected": "using modules.movement\n\nTestEntity {\n    Position: {x: 11.0, y: 22.0}\n    Velocity: {x: 1.0, y: 2.0}\n}"
}
```

## Component Field Format

When defining component fields, use the expected value format:
- Numbers: `10.0`, `42`
- Strings: `"hello world"`
- Booleans: `true`, `false`

## Module Detection

The system automatically detects modules from component names:
- `modules.movement.Position` → `using modules.movement`
- `modules.time.Timer` → `using modules.time`

## Integration

The Test Builder integrates seamlessly with:
- **Existing test execution**: Uses the same backend connection
- **Results viewing**: Automatically navigates to results after test creation
- **File-based workflow**: Generated JSON files work with the upload system

## Navigation

Three main pages are now available:
- **Test Builder**: Create tests visually
- **Upload Tests**: Upload existing JSON files
- **View Results**: Monitor test execution and results