/*
 * Copyright (C) 2011-2025 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdafx.h"
#include "microsip.h"
#include "PasswordDlg.h"
#include "langpack.h"

PasswordDlg::PasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PasswordDlg::IDD, pParent)
{
}

PasswordDlg::~PasswordDlg()
{
}

void PasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PASSWORD, code);
}

BOOL PasswordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	TranslateDialog(this->m_hWnd);
	GetDlgItem(IDC_PASSWORD)->SetFocus();
	return FALSE;
}

BEGIN_MESSAGE_MAP(PasswordDlg, CDialog)
	ON_BN_CLICKED(IDOK, &PasswordDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &PasswordDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

void PasswordDlg::OnBnClickedOk()
{
	if (!UpdateData(TRUE)) {
		return;
	}
	code.Trim();
	EndDialog(IDOK);
}

void PasswordDlg::OnBnClickedCancel()
{
	EndDialog(IDCANCEL);
}
