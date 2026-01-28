<!-- Copy this template and post as a comment on your PR to explain build status -->

## 🔨 Build Status

### Automatic Builds

This PR triggers automatic builds via GitHub Actions. You can track build status here:

- **Quick Build (Linux 64-bit):** [![Build Status](../../actions/workflows/build-quick-test.yml/badge.svg?branch=YOUR_BRANCH_NAME)](../../actions/workflows/build-quick-test.yml)
- **Multi-Platform Build:** [![Build Status](../../actions/workflows/build-multiplatform.yml/badge.svg?branch=YOUR_BRANCH_NAME)](../../actions/workflows/build-multiplatform.yml)

### 📥 Download Artifacts

After builds complete successfully:

1. Go to the [Actions tab](../../actions)
2. Click on the latest successful workflow run (green ✓)
3. Scroll to the **Artifacts** section
4. Download the artifacts you need

**Available artifacts:**
- `xmod-linux64-quick` - Quick test build (Linux 64-bit only) - *Available for 7 days*
- `xmod-multiplatform-release` - Complete multi-platform package - *Available for 90 days*

### 🚀 Manual Build Trigger

You can trigger builds manually without pushing new commits:

1. Go to [Actions](../../actions) → Select workflow
2. Click **"Run workflow"** button
3. Choose this branch: `YOUR_BRANCH_NAME`
4. Click **"Run workflow"**

### ✅ Build Verification Checklist

- [ ] Quick Build passing (Linux 64-bit)
- [ ] Multi-Platform Build passing (all platforms)
- [ ] Artifacts downloaded and tested locally
- [ ] No build warnings in logs
- [ ] Cross-platform compatibility verified

### 📚 Need Help?

See [Using GitHub Actions Guide](../.github/USING_GITHUB_ACTIONS.md) for detailed instructions on:
- Downloading artifacts
- Triggering manual builds
- Understanding build status
- Troubleshooting build issues

---

**Note:** Builds run automatically on every push. You don't need to close this PR to get builds!
