# Historical archive: 2026-11-18

These are superseded reports, closed investigation data and old raw run
evidence, plus original control-document snapshots before R10 consolidation.
They are historical evidence, not a second active checkout or installation.

[manifest.json](manifest.json) records every original path, archive path,
byte size and SHA256. Files were moved individually after checking both
resolved paths stay inside the workspace; destinations were not overwritten.
Each archived file was hashed before and after moving/copying.

289 files moved; 14 original control documents snapshotted.
Total preserved: 4,374,210,881 bytes.
No data was deleted or compressed; disk space is not reclaimed.
Current source, ASIs, INIs and named verification inputs remain protected.

To find a historical file, use its original path in the manifest or
`scripts/workspace_paths.py` from the workspace. Restore specific prerequisites
to their original paths before running a pinned historical workflow. Avoid
overwriting newer files when restoring. Snapshot documents are under
[snapshots](snapshots/); their old next steps are not today's task list.

The supported entry points are [START_HERE](../../START_HERE.md),
[NEXT_ACTION](../../NEXT_ACTION.md) and
[CONFIGURATION](../../CONFIGURATION.md). The cleanup's read-only checker is
`python scripts/verify_workspace_cleanup.py`.

Full dumps/logs, binary extracts and backup ZIPs stay local via ignore rules.
No changes were made to the pre-existing 2026-11-06 archive.

## Published subset

[publication.json](publication.json) lists which manifest entries are included
in this repository and which remain local. The published subset preserves
previously tracked reports/metadata and the original control snapshots; it
does not upload the complete 4.1 GiB archive. Old raw logs previously tracked
are removed from the current tree but remain in earlier Git commits; no Git
history is rewritten. Full cleanup verification requires the local-only files
and fixture prerequisites. Dated snapshots retain their original instructions
as historical evidence, not current tasks.
