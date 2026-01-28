# Using GitHub Actions for xmod

This guide explains how to use GitHub Actions to build xmod without closing or merging pull requests.

## Available Workflows

### 1. Quick Build (Linux 64-bit only)
**File:** `.github/workflows/build-quick-test.yml`

**Purpose:** Fast testing and development builds

**Triggers:**
- Automatically on push to branches: `main`, `develop`, `feature/*`, `fix/*`, `copilot/*`
- Automatically on pull requests to `main` or `develop`
- Manually via "Run workflow" button

**Build time:** ~3-5 minutes  
**Artifacts:** Linux 64-bit binaries (qagame, cgame, ui)  
**Retention:** 7 days

**Use this for:**
- Quick testing of code changes
- PR development and iteration
- Verifying builds before multi-platform testing

### 2. Build Multi-Platform
**File:** `.github/workflows/build-multiplatform.yml`

**Purpose:** Complete multi-platform builds for all supported targets

**Triggers:**
- Automatically on push to `main` or `develop`
- Automatically on pull requests to `main` or `develop`
- Manually via "Run workflow" button

**Build time:** ~15-30 minutes  
**Platforms:**
- Linux (32-bit, 64-bit, ARM64)
- Windows (32-bit, 64-bit)
- macOS (Universal binaries)
- Android (arm64-v8a, armeabi-v7a, x86, x86_64)

**Artifacts:** Complete release package with all platforms  
**Retention:** 90 days (default)

**Use this for:**
- Final testing before release
- Verifying cross-platform compatibility
- Creating distribution packages

### 3. Build and Release
**File:** `.github/workflows/build-release.yml`

**Purpose:** Create GitHub releases with pre-built binaries

**Triggers:**
- Manually only (workflow_dispatch)

**Use this for:**
- Creating official releases
- Publishing pre-release builds
- Distributing stable versions

## How to Use GitHub Actions

### Automatic Builds on Pull Requests

**You DO NOT need to close the PR to get builds!**

1. Create your pull request targeting `main` or `develop`
2. Push commits to your PR branch
3. GitHub Actions automatically builds on every push
4. Download artifacts from the Actions tab (see below)

### Manual Builds

You can trigger builds manually for any branch:

1. Go to your repository on GitHub
2. Click the **"Actions"** tab at the top
3. Select the workflow you want to run (left sidebar):
   - "Quick Build (Linux 64-bit only)" - for fast testing
   - "Build Multi-Platform" - for complete builds
   - "Build and Release" - for releases only
4. Click the **"Run workflow"** button (right side)
5. Select your branch from the dropdown
6. Click **"Run workflow"** to start

### Downloading Build Artifacts

After a workflow completes:

1. Go to the **"Actions"** tab
2. Click on the completed workflow run (green checkmark ✓)
3. Scroll down to the **"Artifacts"** section at the bottom
4. Click on the artifact name to download (e.g., `xmod-linux64-quick`)
5. Extract the ZIP file to get your compiled binaries

**Note:** Artifacts expire after their retention period (7-90 days depending on workflow).

## Workflow Selection Guide

| Scenario | Recommended Workflow | Why |
|----------|---------------------|-----|
| Testing code changes on PR | Quick Build | Fast feedback (3-5 min) |
| Verifying cross-platform compatibility | Build Multi-Platform | Tests all platforms |
| Creating a release | Build and Release | Creates GitHub release |
| Debugging build issues | Quick Build | Fast iteration |
| Pre-merge validation | Build Multi-Platform | Ensures nothing breaks |

## Understanding Build Status

### Status Indicators

- ✓ **Green checkmark** - Build succeeded, artifacts available
- ⊗ **Red X** - Build failed, check logs
- ⟳ **Orange circle** - Build in progress
- ○ **Gray circle** - Build queued/pending

### Viewing Build Logs

If a build fails:

1. Click on the failed workflow run
2. Click on the failed job name
3. Expand the step that failed (red X)
4. Read the error messages in the log

Common issues:
- **Compilation errors** - Fix syntax/logic errors in code
- **Missing dependencies** - Workflow will install them automatically
- **Platform-specific issues** - Check platform-specific code paths

## Branch Protection and Required Checks

Some branches may require successful builds before merging:

- The **Quick Build** workflow runs on all PRs for fast validation
- The **Build Multi-Platform** workflow may be required for `main` branch
- Check with repository maintainers for specific requirements

## Tips and Best Practices

### For Pull Request Authors

1. **Use Quick Build for iteration** - It's faster and good enough for most testing
2. **Run Multi-Platform before requesting review** - Ensures cross-platform compatibility
3. **Keep PRs open** - You can get unlimited builds without closing
4. **Check artifacts before requesting review** - Verify binaries work as expected

### For Reviewers

1. **Download and test artifacts** - Don't just review code, test the build
2. **Check workflow logs** - Look for warnings even if build succeeded
3. **Verify all platforms built** - Especially for platform-specific changes

### For Maintainers

1. **Quick Build on feature branches** - Fast CI feedback
2. **Multi-Platform on develop/main** - Comprehensive validation
3. **Build and Release for tags** - Official distributions

## Troubleshooting

### "Workflow not running on my PR"

Check:
- Is your PR targeting `main` or `develop`?
- Did you push commits after creating the PR?
- Check the Actions tab for queued/running workflows

### "Can't find my artifacts"

Check:
- Did the build complete successfully? (green checkmark)
- Have they expired? (Quick Build: 7 days, Multi-Platform: 90 days)
- Are you looking at the right workflow run?

### "Build is stuck/taking too long"

- Quick Build should complete in 3-5 minutes
- Multi-Platform should complete in 15-30 minutes
- If stuck longer, cancel and re-run the workflow

### "Need to trigger a rebuild without new commits"

1. Go to Actions tab
2. Select the workflow
3. Click "Run workflow" button
4. Choose your branch
5. Click "Run workflow"

## Example: Testing a PR

```bash
# 1. Create feature branch
git checkout -b feature/my-feature

# 2. Make changes
vim src/game/g_client.cpp

# 3. Commit and push
git commit -am "Add new feature"
git push origin feature/my-feature

# 4. Create PR on GitHub
# → Quick Build runs automatically

# 5. Download artifacts from Actions tab
# → Test the binaries locally

# 6. Make more changes if needed
git commit -am "Fix issue found in testing"
git push

# → Quick Build runs again automatically

# 7. When ready for review, manually trigger Multi-Platform build
# → Go to Actions → Build Multi-Platform → Run workflow

# 8. After review, merge PR
# → PR can stay open the entire time!
```

## Questions?

- Check existing workflow runs for examples
- Ask in PR comments if you need help with builds
- Repository maintainers can help with workflow issues

---

**Remember:** You never need to close a PR to get builds. GitHub Actions runs on open PRs!
