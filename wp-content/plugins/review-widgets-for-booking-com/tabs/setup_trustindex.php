<?php
defined( 'ABSPATH' ) or die( 'No script kiddies please!' );
?>
<div id="tab-setup_trustindex">
<div class="ti-box">
<div class="ti-row">
<div class="ti-col-12">
<h1><?php echo TrustindexPlugin_booking::___('Skyrocket Your Sales with Customer Reviews'); ?></h1>
<h2>
<?php echo TrustindexPlugin_booking::___('%s+ WordPress websites use Trustindex to embed reviews fast and easily.', [ '100.000' ]); ?><br />
<?php echo TrustindexPlugin_booking::___('Increase SEO, trust and sales using customer reviews.'); ?>
</h2>
<h3><?php echo TrustindexPlugin_booking::___('Top Features'); ?></h3>
<ul class="ti-check">
<li><?php echo TrustindexPlugin_booking::___('Display unlimited number of reviews'); ?></li>
<li><?php echo TrustindexPlugin_booking::___('Create unlimited number of widgets'); ?></li>
<li><?php echo TrustindexPlugin_booking::___("%d review platforms", [ 58 ]); ?></li>
<li><?php echo TrustindexPlugin_booking::___('Mix reviews from different platforms'); ?></li>
<li><?php echo TrustindexPlugin_booking::___('Get more reviews'); ?></li>
<li><?php echo TrustindexPlugin_booking::___('Manage all reviews in one place'); ?></li>
<li><?php echo TrustindexPlugin_booking::___('Automatically update with NEW reviews'); ?></li>

</ul>
<a class="btn-text btn-lg arrow-btn" href="https://www.trustindex.io/ti-redirect.php?a=sys&c=wp-booking-3" target="_blank"><?php echo TrustindexPlugin_booking::___('Create a Free Trustindex Account for More Features'); ?></a>
<div class="ti-notice notice-success ti-special-offer">
<img src="<?php echo $trustindex_pm_booking->get_plugin_file_url('static/img/special_30.jpg'); ?>">
<p><?php echo TrustindexPlugin_booking::___('Now we offer you a 30%% discount off your subscription! Create your free account and benefit from the onboarding discount now!'); ?></p>
<div class="clear"></div>
</div>
</div>

</div>
</div>
</div>
