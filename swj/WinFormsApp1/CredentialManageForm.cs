using System;
using System.Data;
using System.Windows.Forms;
using Microsoft.Data.SqlClient;

namespace WinFormsApp1
{
    public partial class CredentialManageForm : Form
    {
        private readonly string connStr = @"Data Source=xiaoxi;Initial Catalog=MyDatabase1;Integrated Security=True;TrustServerCertificate=True;";

        public CredentialManageForm()
        {
            InitializeComponent();
        }

        private void CredentialManageForm_Load(object sender, EventArgs e)
        {
            UIHelper.StyleForm(this);
            StyleControls();

            cmbCredentialType.Items.Clear();
            cmbCredentialType.Items.Add("CARD");
            cmbCredentialType.Items.Add("FINGER");
            cmbCredentialType.Items.Add("FACE");
            cmbCredentialType.SelectedIndex = 0;

            chkCredentialEnabled.Checked = true;
            txtCredentialValue.CharacterCasing = CharacterCasing.Upper;

            LoadAccessUsers();
            LoadCredential();
        }

        private void StyleControls()
        {
            UIHelper.StyleLabel(label1);
            UIHelper.StyleLabel(label2);
            UIHelper.StyleLabel(label3);
            UIHelper.StyleLabel(label4);
            UIHelper.StyleLabel(label5);
            UIHelper.StyleTextBox(txtCredentialID);
            UIHelper.StyleTextBox(txtCredentialValue);
            UIHelper.StyleComboBox(cmbCredentialType);
            UIHelper.StyleComboBox(cmbAccessUser);
            UIHelper.StyleDataGridView(dgvCredential);
            UIHelper.StyleRoundButton(button1, UIHelper.Colors.Primary, UIHelper.Colors.LightText);
            UIHelper.StyleRoundButton(button2, UIHelper.Colors.Secondary, UIHelper.Colors.LightText);
            UIHelper.StyleRoundButton(button3, UIHelper.Colors.Warning, Color.FromArgb(44, 62, 80));
            UIHelper.StyleRoundButton(button4, UIHelper.Colors.Danger, UIHelper.Colors.LightText);
        }

        private void LoadAccessUsers()
        {
            try
            {
                using (SqlConnection conn = new SqlConnection(connStr))
                {
                    conn.Open();

                    string sql = @"SELECT AccessUserID, UserName
                                   FROM AccessUser
                                   ORDER BY AccessUserID DESC";

                    using (SqlDataAdapter adapter = new SqlDataAdapter(sql, conn))
                    {
                        DataTable dt = new DataTable();
                        adapter.Fill(dt);

                        cmbAccessUser.DataSource = dt;
                        cmbAccessUser.DisplayMember = "UserName";     // 下拉框显示用户名
                        cmbAccessUser.ValueMember = "AccessUserID";   // 实际取用户ID
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("加载用户下拉列表失败：\n" + ex.Message);
            }
        }

        private void LoadCredential()
        {
            try
            {
                using (SqlConnection conn = new SqlConnection(connStr))
                {
                    conn.Open();

                    string sql = @"SELECT CredentialID, AccessUserID, CredentialType, CredentialValue, IsEnabled, CreatedAt
                                   FROM Credential
                                   WHERE 1=1";

                    if (!string.IsNullOrWhiteSpace(txtCredentialValue.Text))
                    {
                        sql += " AND CredentialValue LIKE @CredentialValue";
                    }

                    sql += " ORDER BY CredentialID DESC";

                    using (SqlCommand cmd = new SqlCommand(sql, conn))
                    {
                        if (!string.IsNullOrWhiteSpace(txtCredentialValue.Text))
                        {
                            cmd.Parameters.AddWithValue("@CredentialValue", "%" + txtCredentialValue.Text.Trim() + "%");
                        }

                        using (SqlDataAdapter adapter = new SqlDataAdapter(cmd))
                        {
                            DataTable dt = new DataTable();
                            adapter.Fill(dt);
                            dgvCredential.DataSource = dt;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("加载凭据列表失败：\n" + ex.Message);
            }
        }

        private void ClearInput()
        {
            txtCredentialID.Text = "";
            txtCredentialValue.Text = "";
            cmbCredentialType.SelectedIndex = 0;
            chkCredentialEnabled.Checked = true;

            if (cmbAccessUser.Items.Count > 0)
            {
                cmbAccessUser.SelectedIndex = 0;
            }
        }

        private void dgvCredential_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex < 0) return;

            DataGridViewRow row = dgvCredential.Rows[e.RowIndex];

            txtCredentialID.Text = row.Cells["CredentialID"].Value?.ToString() ?? "";
            txtCredentialValue.Text = row.Cells["CredentialValue"].Value?.ToString() ?? "";
            cmbCredentialType.Text = row.Cells["CredentialType"].Value?.ToString() ?? "CARD";

            bool isEnabled = false;
            bool.TryParse(row.Cells["IsEnabled"].Value?.ToString(), out isEnabled);
            chkCredentialEnabled.Checked = isEnabled;

            string accessUserIdStr = row.Cells["AccessUserID"].Value?.ToString() ?? "";
            if (!string.IsNullOrWhiteSpace(accessUserIdStr))
            {
                cmbAccessUser.SelectedValue = Convert.ToInt32(accessUserIdStr);
            }
        }

        private void btnLoadCredential_Click(object sender, EventArgs e)
        {
            LoadCredential();
        }

        private void btnAddCredential_Click(object sender, EventArgs e)
        {
            if (cmbAccessUser.SelectedValue == null || string.IsNullOrWhiteSpace(txtCredentialValue.Text))
            {
                MessageBox.Show("请选择用户，并填写凭据值！");
                return;
            }

            try
            {
                using (SqlConnection conn = new SqlConnection(connStr))
                {
                    conn.Open();

                    string sql = @"INSERT INTO Credential
                                   (AccessUserID, CredentialType, CredentialValue, IsEnabled)
                                   VALUES
                                   (@AccessUserID, @CredentialType, @CredentialValue, @IsEnabled)";

                    using (SqlCommand cmd = new SqlCommand(sql, conn))
                    {
                        cmd.Parameters.AddWithValue("@AccessUserID", Convert.ToInt32(cmbAccessUser.SelectedValue));
                        cmd.Parameters.AddWithValue("@CredentialType", cmbCredentialType.Text.Trim());
                        cmd.Parameters.AddWithValue("@CredentialValue", txtCredentialValue.Text.Trim().ToUpper());
                        cmd.Parameters.AddWithValue("@IsEnabled", chkCredentialEnabled.Checked);

                        cmd.ExecuteNonQuery();
                    }
                }

                MessageBox.Show("新增凭据成功！");
                ClearInput();
                LoadCredential();
            }
            catch (Exception ex)
            {
                MessageBox.Show("新增凭据失败：\n" + ex.Message);
            }
        }

        private void btnUpdateCredential_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(txtCredentialID.Text))
            {
                MessageBox.Show("请先选择要修改的凭据！");
                return;
            }

            try
            {
                using (SqlConnection conn = new SqlConnection(connStr))
                {
                    conn.Open();

                    string sql = @"UPDATE Credential
                                   SET AccessUserID = @AccessUserID,
                                       CredentialType = @CredentialType,
                                       CredentialValue = @CredentialValue,
                                       IsEnabled = @IsEnabled
                                   WHERE CredentialID = @CredentialID";

                    using (SqlCommand cmd = new SqlCommand(sql, conn))
                    {
                        cmd.Parameters.AddWithValue("@CredentialID", Convert.ToInt32(txtCredentialID.Text));
                        cmd.Parameters.AddWithValue("@AccessUserID", Convert.ToInt32(cmbAccessUser.SelectedValue));
                        cmd.Parameters.AddWithValue("@CredentialType", cmbCredentialType.Text.Trim());
                        cmd.Parameters.AddWithValue("@CredentialValue", txtCredentialValue.Text.Trim().ToUpper());
                        cmd.Parameters.AddWithValue("@IsEnabled", chkCredentialEnabled.Checked);

                        cmd.ExecuteNonQuery();
                    }
                }

                MessageBox.Show("修改凭据成功！");
                ClearInput();
                LoadCredential();
            }
            catch (Exception ex)
            {
                MessageBox.Show("修改凭据失败：\n" + ex.Message);
            }
        }

        private void btnDeleteCredential_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(txtCredentialID.Text))
            {
                MessageBox.Show("请先选择要禁用的凭据！");
                return;
            }

            DialogResult result = MessageBox.Show("确定要禁用该凭据吗？禁用后将不能再用于识别/开门，但历史记录会保留。",
                "确认禁用", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if (result != DialogResult.Yes) return;

            try
            {
                using (SqlConnection conn = new SqlConnection(connStr))
                {
                    conn.Open();

                    string sql = @"UPDATE Credential
                                   SET IsEnabled = 0
                                   WHERE CredentialID = @CredentialID";

                    using (SqlCommand cmd = new SqlCommand(sql, conn))
                    {
                        cmd.Parameters.AddWithValue("@CredentialID", Convert.ToInt32(txtCredentialID.Text));
                        cmd.ExecuteNonQuery();
                    }
                }

                MessageBox.Show("凭据已成功禁用（逻辑删除）！");
                ClearInput();
                LoadCredential();
            }
            catch (Exception ex)
            {
                MessageBox.Show("禁用凭据失败：\n" + ex.Message);
            }
        }
    }
}