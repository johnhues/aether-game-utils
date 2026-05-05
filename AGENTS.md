# Agent Instructions

## Public Naming
User-facing names should not start with a leading underscore. Internal helpers
and implementation details may use leading-underscore naming.

This split is intentional:
- The Doxygen config excludes `ae::_*` symbols from generated docs.
- Public customization seams should use clean `AE_...` macro names.
- Internal `_AE_...` macros and `ae::_...` symbols are implementation details
  and may hide xmacro machinery from generated docs.

When adding a new user-defined customization point, prefer a clean public
`AE_...` macro even if it feeds internal `_AE_...` utilities.

Do not expose xmacro details directly to users when a clean public macro seam is
possible.
