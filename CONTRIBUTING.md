# Contributing to LED Sign Project

Thank you for your interest in contributing to the LED Sign project! This document provides guidelines and instructions for contributing.

## Code of Conduct

By participating in this project, you agree to maintain a respectful and inclusive environment for all contributors.

## How to Contribute

1. Fork the repository
2. Create a new branch for your feature/fix
3. Make your changes
4. Submit a pull request

## Development Setup

1. Install PlatformIO
```bash
pip install platformio
```

2. Clone the repository
```bash
git clone <your-fork-url>
cd LED-Sign-mic
```

3. Build the project
```bash
pio run
```

4. Upload to your device
```bash
pio run --target upload
```

## Code Style Guidelines

- Follow the existing code style
- Use meaningful variable and function names
- Comment your code when necessary
- Keep functions focused and concise
- Use consistent indentation (2 spaces)

## Testing

- Add tests for new features
- Ensure all tests pass before submitting PR
- Test on actual hardware when possible
- Run `pio test` to execute unit tests

## Pull Request Process

1. Update documentation if needed
2. Add tests for new features
3. Ensure CI checks pass
4. Request review from maintainers
5. Address review feedback

## Commit Messages

- Use clear and descriptive commit messages
- Start with a verb (Add, Fix, Update, etc.)
- Reference issues when applicable

Example:
```
Add LED brightness control feature
- Implement brightness adjustment function
- Add web interface control
- Update documentation
Fixes #123
```

## Documentation

- Update README.md for new features
- Add inline code documentation
- Update hardware guide if needed
- Keep setup guide current

## Hardware Modifications

When proposing hardware changes:
- Document parts list
- Provide clear wiring diagrams
- Consider backward compatibility
- Test thoroughly

## Getting Help

- Check existing issues and documentation
- Join project discussions
- Ask questions in pull requests
- Be patient with responses

## License

By contributing, you agree that your contributions will be licensed under the project's existing license. 