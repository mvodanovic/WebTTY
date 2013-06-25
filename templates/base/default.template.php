<!DOCTYPE html>
<html>
<head>
    <title><?php echo htmlspecialchars($pageTitle); ?> - WebTTY</title>
    <!-- script type="text/javascript" src="<?php echo \Config\Specifics\Data::GetItem('APP_REWRITE_BASE'); ?>static/js/jquery-1.8.2.min.js"></script>
                <script type="text/javascript" src="<?php echo \Config\Specifics\Data::GetItem('APP_REWRITE_BASE'); ?>static/js/jquery-ui-1.9.1.sortable.min.js"></script>
                <link rel="stylesheet" type="text/css" href="<?php echo \Config\Specifics\Data::GetItem('APP_REWRITE_BASE'); ?>static/css/jquery-ui-1.9.1.sortable.min.css" / -->
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <?php echo $htmlHead; ?>
</head>
<body>
<?php echo $htmlBody; ?>
</body>
</html>
