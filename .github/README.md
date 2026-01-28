# GitHub Resources for xmod

This directory contains GitHub-specific resources for the xmod project.

## 📁 Directory Contents

### Workflows (`.github/workflows/`)

Automated build and CI/CD pipelines:

- **`build-quick-test.yml`** - Fast Linux 64-bit builds for rapid testing (3-5 min)
- **`build-multiplatform.yml`** - Complete builds for all platforms (15-30 min)
- **`build-release.yml`** - Release builds with GitHub release creation (manual only)

### Documentation

- **[USING_GITHUB_ACTIONS.md](USING_GITHUB_ACTIONS.md)** - Complete guide on using GitHub Actions
  - How to trigger builds on PRs (without closing them!)
  - How to download artifacts
  - Workflow selection guide
  - Troubleshooting tips

### Templates

- **[PR_BUILD_STATUS_TEMPLATE.md](PR_BUILD_STATUS_TEMPLATE.md)** - Template for PR build status comments
  - Copy and paste into PR comments
  - Includes build badges
  - Artifact download instructions
  - Verification checklist

### Agent Instructions

- **`copilot-instructions.md`** - Instructions for GitHub Copilot agents working on this project

## 🚀 Quick Start

### For Pull Request Authors

1. **Create your PR** - No need to close it for builds!
2. **Push commits** - Quick Build runs automatically
3. **Download artifacts** - From the Actions tab
4. **Iterate** - Push more commits as needed

See [USING_GITHUB_ACTIONS.md](USING_GITHUB_ACTIONS.md) for detailed instructions.

### For First-Time Contributors

1. Read [USING_GITHUB_ACTIONS.md](USING_GITHUB_ACTIONS.md)
2. Use the Quick Build workflow for testing
3. Copy [PR_BUILD_STATUS_TEMPLATE.md](PR_BUILD_STATUS_TEMPLATE.md) to your PR
4. Download and test artifacts before requesting review

## 📊 Workflow Comparison

| Workflow | Trigger | Build Time | Platforms | Artifacts Retention |
|----------|---------|------------|-----------|---------------------|
| Quick Build | Auto on all PRs | 3-5 min | Linux 64-bit only | 7 days |
| Multi-Platform | Auto on main/develop PRs | 15-30 min | All platforms | 90 days |
| Build & Release | Manual only | 15-30 min | All platforms | Permanent (in releases) |

## 🔧 Common Tasks

### "I need a quick test build"
→ Push to your PR, Quick Build runs automatically

### "I need to test on Windows/macOS"
→ Actions → Build Multi-Platform → Run workflow

### "I need to create a release"
→ Actions → Build and Release → Run workflow

### "Where are my build artifacts?"
→ Actions tab → Click on workflow run → Scroll to Artifacts section

## 📚 Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Artifact Upload/Download](https://github.com/actions/upload-artifact)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)

## 💡 Tips

- **Use Quick Build for iteration** - It's much faster for testing code changes
- **Run Multi-Platform before requesting review** - Ensures cross-platform compatibility
- **Keep PRs open** - You get unlimited builds without closing or merging
- **Download and test artifacts** - Verify builds work before merging

## ❓ Questions?

- See [USING_GITHUB_ACTIONS.md](USING_GITHUB_ACTIONS.md) for detailed help
- Ask in PR comments if you need assistance
- Check existing workflow runs for examples

---

**Remember:** You never need to close a PR to compile the code. GitHub Actions runs on open PRs!
