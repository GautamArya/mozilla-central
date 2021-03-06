/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsISelectionController.h"
#include "nsIController.h"
#include "nsIControllers.h"
#include "nsIObserver.h"
#include "nsUnicharUtils.h"
#include "nsIFind.h"
#include "nsIWebBrowserFind.h"
#include "nsWeakReference.h"
#include "nsISelection.h"
#include "nsIDOMRange.h"
#include "nsIDocShellTreeItem.h"
#include "nsITypeAheadFind.h"
#include "nsISound.h"

class nsIPresShell;
class nsPresContext;

#define TYPEAHEADFIND_NOTFOUND_WAV_URL \
        "chrome://global/content/notfound.wav"

class nsTypeAheadFind : public nsITypeAheadFind,
                        public nsIObserver,
                        public nsSupportsWeakReference
{
public:
  nsTypeAheadFind();
  virtual ~nsTypeAheadFind();

  NS_DECL_ISUPPORTS
  NS_DECL_NSITYPEAHEADFIND
  NS_DECL_NSIOBSERVER

protected:
  nsresult PrefsReset();

  void SaveFind();
  void PlayNotFoundSound(); 
  nsresult GetWebBrowserFind(nsIDocShell *aDocShell,
                             nsIWebBrowserFind **aWebBrowserFind);

  void RangeStartsInsideLink(nsIDOMRange *aRange, nsIPresShell *aPresShell, 
                             bool *aIsInsideLink, bool *aIsStartingLink);

  void GetSelection(nsIPresShell *aPresShell, nsISelectionController **aSelCon, 
                    nsISelection **aDomSel);
  bool IsRangeVisible(nsIPresShell *aPresShell, nsPresContext *aPresContext,
                        nsIDOMRange *aRange, bool aMustBeVisible, 
                        bool aGetTopVisibleLeaf, nsIDOMRange **aNewRange,
                        bool *aUsesIndependentSelection);
  nsresult FindItNow(nsIPresShell *aPresShell, bool aIsLinksOnly,
                     bool aIsFirstVisiblePreferred, bool aFindPrev,
                     PRUint16* aResult);
  nsresult GetSearchContainers(nsISupports *aContainer,
                               nsISelectionController *aSelectionController,
                               bool aIsFirstVisiblePreferred,
                               bool aFindPrev, nsIPresShell **aPresShell,
                               nsPresContext **aPresContext);

  // Get the pres shell from mPresShell and return it only if it is still
  // attached to the DOM window.
  NS_HIDDEN_(already_AddRefed<nsIPresShell>) GetPresShell();

  // Current find state
  nsString mTypeAheadBuffer;
  nsCString mNotFoundSoundURL;

  // PRBools are used instead of PRPackedBools because the address of the
  // boolean variable is getting passed into a method.
  bool mStartLinksOnlyPref;
  bool mCaretBrowsingOn;
  nsCOMPtr<nsIDOMElement> mFoundLink;     // Most recent elem found, if a link
  nsCOMPtr<nsIDOMElement> mFoundEditable; // Most recent elem found, if editable
  nsCOMPtr<nsIDOMWindow> mCurrentWindow;
  // mLastFindLength is the character length of the last find string.  It is used for
  // disabling the "not found" sound when using backspace or delete
  PRUint32 mLastFindLength;

  // Sound is played asynchronously on some platforms.
  // If we destroy mSoundInterface before sound has played, it won't play
  nsCOMPtr<nsISound> mSoundInterface;
  bool mIsSoundInitialized;
  
  // where selection was when user started the find
  nsCOMPtr<nsIDOMRange> mStartFindRange;
  nsCOMPtr<nsIDOMRange> mSearchRange;
  nsCOMPtr<nsIDOMRange> mStartPointRange;
  nsCOMPtr<nsIDOMRange> mEndPointRange;

  // Cached useful interfaces
  nsCOMPtr<nsIFind> mFind;

  bool mCaseSensitive;

  bool EnsureFind() {
    if (mFind) {
      return true;
    }

    mFind = do_CreateInstance("@mozilla.org/embedcomp/rangefind;1");
    if (!mFind) {
      return false;
    }

    mFind->SetCaseSensitive(mCaseSensitive);
    mFind->SetWordBreaker(nullptr);

    return true;
  }

  nsCOMPtr<nsIWebBrowserFind> mWebBrowserFind;

  // The focused content window that we're listening to and its cached objects
  nsWeakPtr mDocShell;
  nsWeakPtr mPresShell;
  nsWeakPtr mSelectionController;
                                          // Most recent match's controller
};
