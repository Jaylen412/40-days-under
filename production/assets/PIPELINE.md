# 40 Days Under — visual and media pipeline

The authoritative machine-readable request queue is `manifest.json`. It preserves all five original positive and negative prompts, required aspect ratios, durations, camera directions, dependency order, job IDs, output URLs, and identity-reference provenance. The user approved free generations for all eligible assets. All five assets completed in strict order and were visually inspected; `manifest.json` records job IDs, output URLs, measured dimensions/durations, and review caveats. The initial funding-choice preflight submitted no job, and no paid fallback has been used.

## Ordered execution and identity control

1. Submit 7.1 with the exact stored `generation_params` plus the user's explicit `use_unlim` selection. Wait until completed. Inspect front, three-quarter, and rear views for consistent face, hair waves, build, clothing, wristwatch, and thigh tablet. Preserve the original request and returned adjustments; never silently replace a failed job.
2. Create `Christian Thomas — Lab 04` as a Higgsfield character Reference Element with the completed image job ID, `type: image_job`, and returned HTTPS result URL. Record the returned element ID. Only a `completed` element is usable. Processing/IP checking requires another status read; failed elements must not be used.
3. Generate 7.2 using Seedance 2.0 with `<<<ELEMENT_ID>>>` inside its prompt. Prefix: `Preserve the identity of <<<ELEMENT_ID>>> exactly; show one character, not the multi-view reference-sheet layout.` Then append the original prompt, camera directions and negative prompt. Reference Element IDs do not belong in `medias`; the placeholder injects the reference. Wait for completion and check identity and requested five-second, 21:9 framing.
4. Generate 7.3; complete and inspect before 7.4. Generate 7.4; complete and inspect before 7.5. Generate 7.5 and inspect. This conservative completion gate preserves the user's strict order rather than submitting an independent batch.
5. Record actual returned model, framing, duration, URL, status, and adjustments. Failed or rejected output is not delivered as completed work. Any future Christian shot must reuse the same element; environment-only shots need no character reference.

The identity lock is a reusable reference anchor, not a numeric RNG seed or a trained Soul identity. The selected model APIs expose no numeric seed parameter. Reference conditioning can still drift; a human identity review is required before animation/modeling handoff.

## Capability preflight (2026-11-14)

| Asset | Selected model | Requested output | Supported approach |
|---|---|---|---|
| 7.1 | GPT Image 2.6 | 16:9 turnaround image | 2k medium PNG; three orthographic-style views in one sheet |
| 7.2 | Seedance 2.0 | 21:9, 5 seconds | 720p standard mode; supported reusable element placeholder; dolly-in and subtle handheld expressed in prompt |
| 7.3 | GPT Image 2.6 | 21:9 environment image | 2k medium PNG; 24mm low upward view in prompt |
| 7.4 | Seedance 2.5 | 16:9, 6 seconds | 720p text-to-video; crane-down/current-drift in prompt |
| 7.5 | GPT Image 2.6 | 21:9 environment image | 2k medium PNG; 35mm waterline view in prompt |

All requested aspect ratios and durations are declared supported by the current model schemas. The existing free-trial unlimited allowance covers these selected configurations until 2026-11-17T11:17:18Z, but the tool requires the user to choose it explicitly. A paid fallback must never be inferred from a rejected free request. Confirmed submissions and the completed Character Reference Element are recorded in the manifest. No numeric seed has been invented.

Negative prompts are appended verbatim to the main generation prompt because these models do not declare a dedicated negative-prompt parameter. Camera directions are textual requests, not guaranteed deterministic camera splines. The original `8k texture detail` and engine fidelity terms remain aesthetic prompt language; output is selected at 2k for stills and 720p for video, not actual 8k textures or an Unreal render.

## Review and production handoff

- Character: adult age/read, rich brown skin, neat 360 waves, lean proportions, complete feet, three consistent views, left-wrist watch, readable harness silhouette. Reject inconsistent faces or costume continuity rather than claiming a seed lock fixed them.
- Lab: rusted industrial station, murky green freshwater, plausible freshwater ecology, weighted cable routes, low-visibility atmosphere, silhouetted overhead corpses without gore close-ups.
- Motion: one continuous requested movement; stable geometry and identity, no unwanted cuts, correct duration and aspect ratio, no typography/HUD floating outside physical devices.
- Surface: waterline composition, marina/piling silhouettes and fogged city context. Art is fictionalized production concept work, not surveyed Detroit geography.
- Presentation: `generate_image`/`generate_video` already provide live result widgets. Poll with `jobs_wait` (maximum 15 seconds per call, respect returned polling interval). Do not create duplicate widgets merely to refresh jobs.

These are concept/reference images and cinematic motion references. A turnaround sheet is not a rigged 3D character. Production game assets still require mesh topology, UVs, PBR materials, collision, LODs, skeleton/skin weights, animation retargeting, engine import, lighting validation and platform performance budgets. No Image-to-3D or rigged mesh export was executed or claimed.

## Resume without duplicates

Read `manifest.json` before submission. If `job_id` exists, poll that job rather than resubmitting. A timeout after submission means unknown outcome and requires recovery by supported job lookup. This requested five-asset set is complete; do not resubmit it. For any separately authorized continuation with no job and completed predecessors, submit the stored parameters with the approved funding selection. Never resubmit an asset whose job ID is already recorded. Update the manifest immediately after each accepted job and each terminal result.
