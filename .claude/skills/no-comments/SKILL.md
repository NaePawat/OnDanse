---
name: no-comments
description: No unsolicited code comments. Use whenever writing or editing code.
---

## Rule

Don't comment. Names and structure explain the code.

## Never

- Restating code: `// increment counter`
- Narrating changes: `// added to fix bug`, `// changed from X`
- Talking to reviewer/user: `// as requested`
- Banners: `// ===== Helpers =====`
- Docstrings on obvious functions
- Filler: `// rest of code here`
- Commented-out code
- End markers: `} // end if`

## Allowed

- Non-obvious **why**: workaround, hidden constraint, subtle invariant
- Tooling: lint/type pragmas, license headers
- Public API docs, only if the codebase already does it
- User asked for it

Unsure? Leave it out.

## Existing code

- Match surrounding comment density.
- Keep existing comments; fix or remove only ones your change makes wrong.

## Before done

Scan diff, delete comments that break the rule. Rename > comment.
