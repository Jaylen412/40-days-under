# Validation record

Validated on November 14, 2026 with Python 3.13.7 and Apple clang 21.

- Python simulation: 17 tests pass, including both branch budgets, boundary timing, interrupted life support, decompression injury, death precedence, single-slot persistence and immutable terminal archives.
- Production contracts: 38 canonical events, five opening beats totaling 300 seconds, and five ordered asset requests pass cross-file validation.
- Generated C++ profiles match the editable JSON mapping and canonical event set.
- C++17 feedback harness compiles with `-Wall -Wextra -Werror -pedantic` and passes priority, expiry, source, accessibility, lifecycle and bounded-input checks.

Commands:

```sh
python3 -m unittest discover -s simulation -p 'test_*.py'
python3 tools/validate_production.py
python3 hardware/generate_profiles.py --check
c++ -std=c++17 -Wall -Wextra -Werror -pedantic hardware/test_feedback.cpp -o /tmp/40-days-under-feedback-test
/tmp/40-days-under-feedback-test
```

These checks verify reference software and authored contracts. They do not verify an Unreal scene, physical controller, Sony SDK integration, actual diving equipment, spatial enemy AI or an exported 3D mesh. Media completion and review evidence are recorded separately in `assets/manifest.json`.
