using System;
using System.Data;
using System.Windows.Forms;
using Microsoft.Data.SqlClient;

namespace WinFormsApp1
{
    public partial class OperationLogForm : Form
    {
        private readonly string connStr = @"Data Source=xiaoxi;Initial Catalog=MyDatabase1;Integrated Security=True;TrustServerCertificate=True;";

        public OperationLogForm()
        {
            InitializeComponent();
        }

        private void OperationLogForm_Load(object sender, EventArgs e)
        {
            UIHelper.StyleForm(this);
            UIHelper.StyleDataGridView(dgvOperationLog);
            LoadOperationLog();
        }

        private void LoadOperationLog()
        {
            try
            {
                using (SqlConnection conn = new SqlConnection(connStr))
                {
                    conn.Open();

                    string sql = @"SELECT LogID, AdminID, OperationType, OperationContent, OperationTime, OperationIP, TargetUserID
                                   FROM OperationLog
                                   ORDER BY OperationTime DESC";

                    using (SqlDataAdapter adapter = new SqlDataAdapter(sql, conn))
                    {
                        DataTable dt = new DataTable();
                        adapter.Fill(dt);
                        dgvOperationLog.DataSource = dt;
                    }
                }

                FormatOperationLogGrid();
            }
            catch (Exception ex)
            {
                MessageBox.Show("加载系统日志失败：\n" + ex.Message);
            }
        }

        private void FormatOperationLogGrid()
        {
            dgvOperationLog.ReadOnly = true;
            dgvOperationLog.MultiSelect = false;
            dgvOperationLog.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
            dgvOperationLog.RowHeadersVisible = false;
            dgvOperationLog.AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.None;
            dgvOperationLog.AllowUserToAddRows = false;

            if (dgvOperationLog.Columns.Contains("LogID"))
            {
                dgvOperationLog.Columns["LogID"].HeaderText = "日志ID";
                dgvOperationLog.Columns["LogID"].Width = 70;
            }

            if (dgvOperationLog.Columns.Contains("AdminID"))
            {
                dgvOperationLog.Columns["AdminID"].HeaderText = "管理员ID";
                dgvOperationLog.Columns["AdminID"].Width = 80;
            }

            if (dgvOperationLog.Columns.Contains("OperationType"))
            {
                dgvOperationLog.Columns["OperationType"].HeaderText = "操作类型";
                dgvOperationLog.Columns["OperationType"].Width = 90;
            }

            if (dgvOperationLog.Columns.Contains("OperationContent"))
            {
                dgvOperationLog.Columns["OperationContent"].HeaderText = "操作内容";
                dgvOperationLog.Columns["OperationContent"].Width = 220;
            }

            if (dgvOperationLog.Columns.Contains("OperationTime"))
            {
                dgvOperationLog.Columns["OperationTime"].HeaderText = "操作时间";
                dgvOperationLog.Columns["OperationTime"].Width = 150;
                dgvOperationLog.Columns["OperationTime"].DefaultCellStyle.Format = "yyyy-MM-dd HH:mm:ss";
            }

            if (dgvOperationLog.Columns.Contains("OperationIP"))
            {
                dgvOperationLog.Columns["OperationIP"].HeaderText = "IP地址";
                dgvOperationLog.Columns["OperationIP"].Width = 120;
            }

            if (dgvOperationLog.Columns.Contains("TargetUserID"))
            {
                dgvOperationLog.Columns["TargetUserID"].HeaderText = "目标用户ID";
                dgvOperationLog.Columns["TargetUserID"].AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill;
            }
        }

        private void dgvOperationLog_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }
    }
}
