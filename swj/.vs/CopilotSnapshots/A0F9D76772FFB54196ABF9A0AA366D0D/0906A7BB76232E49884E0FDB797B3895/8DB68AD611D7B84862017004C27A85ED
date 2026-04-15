using System;
using System.Data;
using System.Windows.Forms;
using Microsoft.Data.SqlClient;

namespace WinFormsApp1
{
    public partial class AccessRecordForm : Form
    {
        private readonly string connStr = @"Data Source=xiaoxi;Initial Catalog=MyDatabase1;Integrated Security=True;TrustServerCertificate=True;";

        public AccessRecordForm()
        {
            InitializeComponent();
        }

        private void AccessRecordForm_Load(object sender, EventArgs e)
        {
            UIHelper.StyleForm(this);
            UIHelper.StyleRoundButton(btnLoadAccessRecord, UIHelper.Colors.Primary, UIHelper.Colors.LightText);
            UIHelper.StyleDataGridView(dgvAccessRecord);
            LoadAccessRecord();
        }

        private void LoadAccessRecord()
        {
            try
            {
                using (SqlConnection conn = new SqlConnection(connStr))
                {
                    conn.Open();

                    string sql = @"
                        SELECT 
                            ar.RecordID,
                            ar.AccessUserID,
                            au.UserName,
                            ar.CredentialID,
                            c.CredentialType,
                            c.CredentialValue,
                            ar.EventType,
                            ar.Result,
                            ar.EventTime,
                            ar.DevicePort,
                            ar.RawMessage,
                            ar.Remark
                        FROM AccessRecord ar
                        LEFT JOIN AccessUser au ON ar.AccessUserID = au.AccessUserID
                        LEFT JOIN Credential c ON ar.CredentialID = c.CredentialID
                        ORDER BY ar.EventTime DESC";

                    using (SqlDataAdapter adapter = new SqlDataAdapter(sql, conn))
                    {
                        DataTable dt = new DataTable();
                        adapter.Fill(dt);
                        dgvAccessRecord.DataSource = dt;
                    }
                }

                FormatAccessRecordGrid();
            }
            catch (Exception ex)
            {
                MessageBox.Show("加载开门记录失败：\n" + ex.Message);
            }
        }

        private void FormatAccessRecordGrid()
        {
            dgvAccessRecord.ReadOnly = true;
            dgvAccessRecord.MultiSelect = false;
            dgvAccessRecord.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
            dgvAccessRecord.RowHeadersVisible = false;
            dgvAccessRecord.AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.None;
            dgvAccessRecord.AllowUserToAddRows = false;

            if (dgvAccessRecord.Columns.Contains("RecordID"))
            {
                dgvAccessRecord.Columns["RecordID"].HeaderText = "记录ID";
                dgvAccessRecord.Columns["RecordID"].Width = 70;
            }

            if (dgvAccessRecord.Columns.Contains("AccessUserID"))
            {
                dgvAccessRecord.Columns["AccessUserID"].HeaderText = "用户ID";
                dgvAccessRecord.Columns["AccessUserID"].Width = 70;
            }

            if (dgvAccessRecord.Columns.Contains("UserName"))
            {
                dgvAccessRecord.Columns["UserName"].HeaderText = "用户姓名";
                dgvAccessRecord.Columns["UserName"].Width = 90;
            }

            if (dgvAccessRecord.Columns.Contains("CredentialID"))
            {
                dgvAccessRecord.Columns["CredentialID"].HeaderText = "凭据ID";
                dgvAccessRecord.Columns["CredentialID"].Width = 70;
            }

            if (dgvAccessRecord.Columns.Contains("CredentialType"))
            {
                dgvAccessRecord.Columns["CredentialType"].HeaderText = "凭据类型";
                dgvAccessRecord.Columns["CredentialType"].Width = 80;
            }

            if (dgvAccessRecord.Columns.Contains("CredentialValue"))
            {
                dgvAccessRecord.Columns["CredentialValue"].HeaderText = "凭据值";
                dgvAccessRecord.Columns["CredentialValue"].Width = 110;
            }

            if (dgvAccessRecord.Columns.Contains("EventType"))
            {
                dgvAccessRecord.Columns["EventType"].HeaderText = "事件类型";
                dgvAccessRecord.Columns["EventType"].Width = 90;
            }

            if (dgvAccessRecord.Columns.Contains("Result"))
            {
                dgvAccessRecord.Columns["Result"].HeaderText = "结果";
                dgvAccessRecord.Columns["Result"].Width = 70;
            }

            if (dgvAccessRecord.Columns.Contains("EventTime"))
            {
                dgvAccessRecord.Columns["EventTime"].HeaderText = "时间";
                dgvAccessRecord.Columns["EventTime"].Width = 140;
                dgvAccessRecord.Columns["EventTime"].DefaultCellStyle.Format = "yyyy-MM-dd HH:mm:ss";
            }

            if (dgvAccessRecord.Columns.Contains("DevicePort"))
            {
                dgvAccessRecord.Columns["DevicePort"].HeaderText = "串口";
                dgvAccessRecord.Columns["DevicePort"].Width = 70;
            }

            if (dgvAccessRecord.Columns.Contains("RawMessage"))
            {
                dgvAccessRecord.Columns["RawMessage"].HeaderText = "原始报文";
                dgvAccessRecord.Columns["RawMessage"].Width = 130;
            }

            if (dgvAccessRecord.Columns.Contains("Remark"))
            {
                dgvAccessRecord.Columns["Remark"].HeaderText = "备注";
                dgvAccessRecord.Columns["Remark"].AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill;
            }
        }

        private void btnLoadAccessRecord_Click(object sender, EventArgs e)
        {
            LoadAccessRecord();
        }
    }
}
