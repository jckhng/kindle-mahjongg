# Release Notes

## Current Release

Artifact:

```text
release/kindle-mahjongg-extension.zip
```

Screenshot:

```text
screenshots/kindle-mahjongg.png
```

Verify:

```bash
cd release
sha256sum -c SHA256SUMS
```

Current checksum:

```text
db6d04e5b89fe02459946936cf5d814b0d7a161b09c5caccfca1d43b2370df19  kindle-mahjongg-extension.zip
```

Contents:

- ARM hard-float `kindle-mahjongg` executable.
- KUAL extension metadata and launch scripts.
- Mahjongg visual assets copied from GNOME Games.
- Bundled GTK2/Cairo runtime library set copied from the ARM Docker builder.
- License and third-party runtime notices.

Known constraints:

- This is an unofficial derivative/adaptation release, not an official GNOME or
  GnomeGames4Kindle release.
- Requires a jailbroken Kindle with KUAL.
- The standalone tile shuffler does not guarantee every random deal is solvable.
- Kindle home-screen `.sh` tapping is not reliable unless another launcher/file
  association is installed. Use KUAL.

GitHub release upload:

- Upload `release/kindle-mahjongg-extension.zip` as the binary asset.
- Include the checksum from `release/SHA256SUMS` in the release notes.
- Keep the source repository public with `licenses/`, `docs/`, and `assets/`
  intact so recipients can inspect the derivative-work provenance.
