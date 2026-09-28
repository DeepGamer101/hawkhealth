# Getting your own HawkHealth repo

> **Mentor's note.** HawkHealth is a *template*. You don't edit the class copy — you make your
> own private copy, and that becomes your workspace for the whole semester. This is exactly how
> a new engineer gets set up on a real project: you're handed a starting point and you make it
> yours. Ten minutes, one time.

## Steps

1. **Open the template repo** (your instructor will give you the link) and click the green
   **"Use this template" → "Create a new repository."**
2. **Name it** `hawkhealth-<your-last-name>` (e.g., `hawkhealth-broshar`).
3. **Set visibility to Private.** *(Important — your work stays yours.)*
4. Click **Create repository**. GitHub makes you a full, independent copy.
5. **Add your instructor as a collaborator** so they can see your work and grade it:
   **Settings → Collaborators → Add people →** add your instructor's GitHub username.
6. **Confirm CI is running.** Open the **Actions** tab. You should see **"HawkHealth CI"**
   kick off and, after a few minutes, a green ✓. *(If you see a banner asking to enable
   workflows, click to enable it, then make any small commit to trigger a run.)*
7. **Clone it to your laptop** (GitHub Desktop: *File → Clone repository*, or
   `git clone <your-repo-url>`).

## You're set when…

- [ ] You have a **private** `hawkhealth-<name>` repo,
- [ ] its **Actions tab shows a green ✓** (CI passed on the starting code),
- [ ] your **instructor is a collaborator**, and
- [ ] you've **cloned it** locally.

## Now start Week 1

Open **[`docs/week-01.md`](week-01.md)** and follow the lab.

---

### How you'll work all semester (the rhythm)

You never commit straight to `main`. For each week's task:

1. make a **branch**, do the work,
2. **push** and open a **pull request**,
3. watch **CI** run — green means your change builds and passes the tests,
4. merge when it's green.

That branch → PR → CI-green → merge loop is the professional habit this course is built on.
"It compiles" isn't "done" — **done is when CI is green.**
